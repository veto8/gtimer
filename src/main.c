#include "gtimer-window.h"
#include "report-window.h"
#include "../core/db-manager.h"
#include "../core/task-list-model.h"
#include "../core/timer-service.h"
#include "../core/idle-monitor.h"
#include "../core/timer-utils.h"
#include "../core/report-generator.h"

typedef struct {
  GTimerDBManager *db_manager;
  GTimerTaskListModel *task_list_model;
  GTimerTimerService *timer_service;
  GTimerIdleMonitor *idle_monitor;
} GTimerApp;

static void
on_timer_tick (GTimerTimerService *service, gint64 elapsed, gpointer user_data)
{
  (void)elapsed;
  GtkApplication *app = GTK_APPLICATION (user_data);
  static gint64 last_notif_time = 0;
  gint64 now = time (NULL);
  
  // Update notification if app is in background, throttle to once every 10s
  if (!gtk_application_get_active_window (app)) {
    if (now - last_notif_time >= 10) {
      GTimerTask *task = gtimer_timer_service_get_active_task (service);
      if (task) {
        GNotification *notif = g_notification_new ("GTimer Running");
        char *body = g_strdup_printf ("Timing: %s", gtimer_task_get_name (task));
        g_notification_set_body (notif, body);
        g_free (body);
        g_notification_set_default_action (notif, "app.activate");
        g_application_send_notification (G_APPLICATION (app), "timer-active", notif);
        g_object_unref (notif);
        last_notif_time = now;
      }
    }
  } else {
    // App is active, withdraw notification
    g_application_withdraw_notification (G_APPLICATION (app), "timer-active");
    last_notif_time = 0;
  }
}

static void
activate (GtkApplication *app,
          gpointer        user_data)
{
  GTimerApp *gtimer_app = user_data;
  GTimerWindow *window;

  // Ensure icons from resources are found by the icon theme
  GtkIconTheme *theme = gtk_icon_theme_get_for_display (gdk_display_get_default ());
  gtk_icon_theme_add_resource_path (theme, "/us/k5n/GTimer/icons");
  
  // Set default icon name for all windows
  gtk_window_set_default_icon_name ("us.k5n.GTimer");

  // Load CSS
  GtkCssProvider *provider = gtk_css_provider_new ();
  gtk_css_provider_load_from_resource (provider, "/us/k5n/GTimer/style.css");
  gtk_style_context_add_provider_for_display (gdk_display_get_default (),
                                             GTK_STYLE_PROVIDER (provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref (provider);

  window = gtimer_window_new (app);
  
  gtimer_window_set_task_list_model (window, gtimer_app->task_list_model);
  gtimer_window_set_timer_service (window, gtimer_app->timer_service);

  // Connect to tick for background notifications
  g_signal_connect (gtimer_app->timer_service, "tick", G_CALLBACK (on_timer_tick), app);

  g_printerr ("DEBUG: activate: Refreshing model\n");
  gtimer_task_list_model_refresh (gtimer_app->task_list_model);

  gtk_window_present (GTK_WINDOW (window));
}

static GFile *last_save_folder = NULL;

static void
on_html_save_response (GtkNativeDialog *native, int response_id, gpointer user_data)
{
  char *report_text = user_data;
  if (response_id == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (native);
    GFile *file = gtk_file_chooser_get_file (chooser);
    if (file) {
      // Save the folder for next time
      g_clear_object (&last_save_folder);
      last_save_folder = g_file_get_parent (file);

      GError *err = NULL;
      if (g_file_replace_contents (file, report_text, strlen (report_text), NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, &err)) {
        char *uri = g_file_get_uri (file);
        gtk_show_uri (gtk_native_dialog_get_transient_for (native), uri, GDK_CURRENT_TIME);
        g_free (uri);
      } else {
        g_warning ("Failed to save HTML report: %s", err->message);
        g_clear_error (&err);
      }
      g_object_unref (file);
    }
  }
  g_free (report_text);
  g_object_unref (native);
}

static void
on_report_dialog_response (GtkDialog *dialog, int response_id, gpointer user_data)
{
  if (response_id == GTK_RESPONSE_OK) {
    GtkApplication *app = GTK_APPLICATION (user_data);
    GTimerApp *gtimer_app = g_object_get_data (G_OBJECT (app), "gtimer-app");
    GtkWindow *parent = gtk_application_get_active_window (app);
    GtkWidget *content_area = gtk_dialog_get_content_area (dialog);
    GtkDropDown *round_dropdown = g_object_get_data (G_OBJECT (content_area), "round-dropdown");
    GtkDropDown *format_dropdown = g_object_get_data (G_OBJECT (content_area), "format-dropdown");
    
    int rounding = 0;
    guint selected = gtk_drop_down_get_selected (round_dropdown);
    switch (selected) {
      case 1: rounding = 1; break;
      case 2: rounding = 5; break;
      case 3: rounding = 10; break;
      case 4: rounding = 15; break;
      case 5: rounding = 30; break;
      case 6: rounding = 60; break;
      default: rounding = 0; break;
    }

    GTimerReportFormat format = GTIMER_REPORT_TEXT;
    if (gtk_drop_down_get_selected (format_dropdown) == 1) {
      format = GTIMER_REPORT_HTML;
    }

    GDateTime *now = g_date_time_new_now_local ();
    GDateTime *start_date = g_date_time_ref (now);
    GDateTime *end_date = g_date_time_ref (now);
    
    guint range_idx = gtk_drop_down_get_selected (g_object_get_data (G_OBJECT (content_area), "range-dropdown"));
    
    // Simplistic range calculation
    switch (range_idx) {
      case 0: // Today
        break;
      case 1: // This Week
        start_date = g_date_time_add_days (now, -(int)g_date_time_get_day_of_week (now) + 1);
        break;
      case 2: // Last Week
        {
          GDateTime *last_week = g_date_time_add_days (now, -7);
          start_date = g_date_time_add_days (last_week, -(int)g_date_time_get_day_of_week (last_week) + 1);
          end_date = g_date_time_add_days (start_date, 6);
          g_date_time_unref (last_week);
        }
        break;
      case 5: // This Month
        start_date = g_date_time_new_local (g_date_time_get_year (now), g_date_time_get_month (now), 1, 0, 0, 0);
        break;
      case 7: // This Year
        start_date = g_date_time_new_local (g_date_time_get_year (now), 1, 1, 0, 0, 0);
        break;
      default:
        // Handle other ranges if needed
        break;
    }

    char *report_text = gtimer_report_generate (gtimer_app->db_manager,
                                                GTIMER_REPORT_DAILY, // Not used in engine anymore
                                                format,
                                                start_date, end_date, NULL, rounding);
    
    if (format == GTIMER_REPORT_HTML) {
      GtkFileChooserNative *native = gtk_file_chooser_native_new ("Save HTML Report",
                                                                  parent,
                                                                  GTK_FILE_CHOOSER_ACTION_SAVE,
                                                                  "_Save",
                                                                  "_Cancel");
      
      char *date_slug = g_date_time_format (now, "%Y%m%d");
      char *default_filename = g_strdup_printf ("gtimer-report-%s.html", date_slug);
      gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (native), default_filename);
      g_free (default_filename);
      g_free (date_slug);

      if (last_save_folder) {
        gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (native), last_save_folder, NULL);
      }

      g_signal_connect (native, "response", G_CALLBACK (on_html_save_response), g_strdup (report_text));
      gtk_native_dialog_show (GTK_NATIVE_DIALOG (native));
    } else {
      GTimerReportWindow *report_window = gtimer_report_window_new (parent, "Report", report_text);
      gtk_window_present (GTK_WINDOW (report_window));
    }
    
    g_free (report_text);
    g_date_time_unref (now);
    g_date_time_unref (start_date);
    g_date_time_unref (end_date);
  }
  gtk_window_destroy (GTK_WINDOW (dialog));
}

static void
on_report_action (GSimpleAction *action,
                  GVariant      *parameter,
                  gpointer       user_data)
{
  (void)action;
  (void)parameter;
  GtkApplication *app = GTK_APPLICATION (user_data);
  GtkWindow *parent = gtk_application_get_active_window (app);
  GTimerApp *gtimer_app = g_object_get_data (G_OBJECT (app), "gtimer-app");
  
  GtkWidget *dialog = gtk_dialog_new_with_buttons ("Generate Report",
                                                   parent,
                                                   GTK_DIALOG_MODAL | GTK_DIALOG_USE_HEADER_BAR,
                                                   "_Cancel", GTK_RESPONSE_CANCEL,
                                                   "_Generate", GTK_RESPONSE_OK,
                                                   NULL);
  
  GtkWidget *content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
  gtk_box_set_spacing (GTK_BOX (content_area), 12);
  gtk_widget_set_margin_start (content_area, 12);
  gtk_widget_set_margin_end (content_area, 12);
  gtk_widget_set_margin_top (content_area, 12);
  gtk_widget_set_margin_bottom (content_area, 12);

  // Report Type
  GtkWidget *type_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_box_append (GTK_BOX (type_box), gtk_label_new ("Report Type:"));
  GtkStringList *type_list = gtk_string_list_new ((const char *[]) {"Daily", "Weekly", "Monthly", "Yearly", NULL});
  GtkDropDown *type_dropdown = GTK_DROP_DOWN (gtk_drop_down_new (G_LIST_MODEL (type_list), NULL));
  gtk_box_append (GTK_BOX (type_box), GTK_WIDGET (type_dropdown));
  gtk_box_append (GTK_BOX (content_area), type_box);

  // Time Range
  GtkWidget *range_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_box_append (GTK_BOX (range_box), gtk_label_new ("Time Range:"));
  GtkStringList *range_list = gtk_string_list_new ((const char *[]) {
    "Today", "This Week", "Last Week", "This & Last Week", "Last Two Weeks", 
    "This Month", "Last Month", "This Year", "Last Year", NULL
  });
  GtkDropDown *range_dropdown = GTK_DROP_DOWN (gtk_drop_down_new (G_LIST_MODEL (range_list), NULL));
  gtk_box_append (GTK_BOX (range_box), GTK_WIDGET (range_dropdown));
  gtk_box_append (GTK_BOX (content_area), range_box);

  // Rounding
  GtkWidget *round_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_box_append (GTK_BOX (round_box), gtk_label_new ("Rounding:"));
  GtkStringList *round_list = gtk_string_list_new ((const char *[]) {
    "None", "1 Minute", "5 Minutes", "10 Minutes", "15 Minutes", "30 Minutes", "1 Hour", NULL
  });
  GtkDropDown *round_dropdown = GTK_DROP_DOWN (gtk_drop_down_new (G_LIST_MODEL (round_list), NULL));
  gtk_box_append (GTK_BOX (round_box), GTK_WIDGET (round_dropdown));
  gtk_box_append (GTK_BOX (content_area), round_box);

  // Format
  GtkWidget *format_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_box_append (GTK_BOX (format_box), gtk_label_new ("Format:"));
  GtkStringList *format_list = gtk_string_list_new ((const char *[]) {"Plain Text", "HTML", NULL});
  GtkDropDown *format_dropdown = GTK_DROP_DOWN (gtk_drop_down_new (G_LIST_MODEL (format_list), NULL));
  gtk_box_append (GTK_BOX (format_box), GTK_WIDGET (format_dropdown));
  gtk_box_append (GTK_BOX (content_area), format_box);

  // Task Selection (Checkbox list)
  gtk_box_append (GTK_BOX (content_area), gtk_label_new ("Tasks:"));
  GtkWidget *scrolled = gtk_scrolled_window_new ();
  gtk_widget_set_size_request (scrolled, -1, 200);
  gtk_box_append (GTK_BOX (content_area), scrolled);
  
  GtkWidget *task_list = gtk_list_box_new ();
  gtk_list_box_set_selection_mode (GTK_LIST_BOX (task_list), GTK_SELECTION_NONE);
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled), task_list);

  GListModel *model = gtimer_task_list_model_get_model (gtimer_app->task_list_model);
  guint n_items = g_list_model_get_n_items (model);
  for (guint i = 0; i < n_items; i++) {
    GTimerTask *task = GTIMER_TASK (g_list_model_get_item (model, i));
    GtkWidget *row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
    GtkWidget *check = gtk_check_button_new ();
    gtk_check_button_set_active (GTK_CHECK_BUTTON (check), TRUE);
    gtk_box_append (GTK_BOX (row), check);
    gtk_box_append (GTK_BOX (row), gtk_label_new (gtimer_task_get_name (task)));
    gtk_list_box_append (GTK_LIST_BOX (task_list), row);
    g_object_set_data (G_OBJECT (row), "task-id", GINT_TO_POINTER (gtimer_task_get_id (task)));
    g_object_set_data (G_OBJECT (row), "check", check);
    g_object_unref (task);
  }

  g_object_set_data (G_OBJECT (content_area), "range-dropdown", range_dropdown);
  g_object_set_data (G_OBJECT (content_area), "round-dropdown", round_dropdown);
  g_object_set_data (G_OBJECT (content_area), "format-dropdown", format_dropdown);
  g_object_set_data (G_OBJECT (content_area), "task-list", task_list);

  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);
  
  // For now, reuse the simple daily report generation logic when Generate is clicked
  g_signal_connect (dialog, "response", G_CALLBACK (on_report_dialog_response), app);
  gtk_widget_show (dialog);
}

static void
on_preferences_action (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void)action; (void)parameter;
  GtkApplication *app = GTK_APPLICATION (user_data);
  GtkWindow *parent = gtk_application_get_active_window (app);
  GSettings *settings = g_settings_new ("us.k5n.GTimer");

  GtkWidget *dialog = adw_preferences_window_new ();
  gtk_window_set_transient_for (GTK_WINDOW (dialog), parent);

  // General Page
  AdwPreferencesPage *page = ADW_PREFERENCES_PAGE (adw_preferences_page_new ());
  adw_preferences_page_set_title (page, "General");
  adw_preferences_page_set_icon_name (page, "preferences-system-symbolic");
  adw_preferences_window_add (ADW_PREFERENCES_WINDOW (dialog), page);

  AdwPreferencesGroup *group = ADW_PREFERENCES_GROUP (adw_preferences_group_new ());
  adw_preferences_group_set_title (group, "Behavior");
  adw_preferences_page_add (page, group);

  // Auto Save
  AdwActionRow *auto_save_row = ADW_ACTION_ROW (adw_action_row_new ());
  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (auto_save_row), "Auto Save");
  GtkWidget *auto_save_switch = gtk_switch_new ();
  gtk_widget_set_valign (auto_save_switch, GTK_ALIGN_CENTER);
  adw_action_row_add_suffix (auto_save_row, auto_save_switch);
  adw_preferences_group_add (group, GTK_WIDGET (auto_save_row));
  g_settings_bind (settings, "auto-save", auto_save_switch, "active", G_SETTINGS_BIND_DEFAULT);

  // Animate
  AdwActionRow *animate_row = ADW_ACTION_ROW (adw_action_row_new ());
  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (animate_row), "Animate Running Tasks");
  GtkWidget *animate_switch = gtk_switch_new ();
  gtk_widget_set_valign (animate_switch, GTK_ALIGN_CENTER);
  adw_action_row_add_suffix (animate_row, animate_switch);
  adw_preferences_group_add (group, GTK_WIDGET (animate_row));
  g_settings_bind (settings, "animate-running-tasks", animate_switch, "active", G_SETTINGS_BIND_DEFAULT);

  // Resume
  AdwActionRow *resume_row = ADW_ACTION_ROW (adw_action_row_new ());
  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (resume_row), "Resume Timing on Startup");
  GtkWidget *resume_switch = gtk_switch_new ();
  gtk_widget_set_valign (resume_switch, GTK_ALIGN_CENTER);
  adw_action_row_add_suffix (resume_row, resume_switch);
  adw_preferences_group_add (group, GTK_WIDGET (resume_row));
  g_settings_bind (settings, "resume-on-startup", resume_switch, "active", G_SETTINGS_BIND_DEFAULT);

  // Idle Detection Page
  page = ADW_PREFERENCES_PAGE (adw_preferences_page_new ());
  adw_preferences_page_set_title (page, "Idle Detection");
  adw_preferences_page_set_icon_name (page, "preferences-desktop-screensaver-symbolic");
  adw_preferences_window_add (ADW_PREFERENCES_WINDOW (dialog), page);

  group = ADW_PREFERENCES_GROUP (adw_preferences_group_new ());
  adw_preferences_page_add (page, group);

  // Idle Enable
  AdwActionRow *idle_enable_row = ADW_ACTION_ROW (adw_action_row_new ());
  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (idle_enable_row), "Enable Idle Detection");
  GtkWidget *idle_enable_switch = gtk_switch_new ();
  gtk_widget_set_valign (idle_enable_switch, GTK_ALIGN_CENTER);
  adw_action_row_add_suffix (idle_enable_row, idle_enable_switch);
  adw_preferences_group_add (group, GTK_WIDGET (idle_enable_row));
  g_settings_bind (settings, "enable-idle-detection", idle_enable_switch, "active", G_SETTINGS_BIND_DEFAULT);

  // Idle Threshold
  AdwActionRow *idle_threshold_row = ADW_ACTION_ROW (adw_action_row_new ());
  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (idle_threshold_row), "Idle Threshold (minutes)");
  GtkWidget *idle_threshold_spin = gtk_spin_button_new (gtk_adjustment_new (15, 1, 120, 1, 10, 0), 1, 0);
  gtk_widget_set_valign (idle_threshold_spin, GTK_ALIGN_CENTER);
  adw_action_row_add_suffix (idle_threshold_row, idle_threshold_spin);
  adw_preferences_group_add (group, GTK_WIDGET (idle_threshold_row));
  g_settings_bind (settings, "idle-threshold", idle_threshold_spin, "value", G_SETTINGS_BIND_DEFAULT);

  // Time Page
  page = ADW_PREFERENCES_PAGE (adw_preferences_page_new ());
  adw_preferences_page_set_title (page, "Time");
  adw_preferences_page_set_icon_name (page, "preferences-system-time-symbolic");
  adw_preferences_window_add (ADW_PREFERENCES_WINDOW (dialog), page);

  group = ADW_PREFERENCES_GROUP (adw_preferences_group_new ());
  adw_preferences_page_add (page, group);

  // Midnight Offset
  AdwActionRow *midnight_row = ADW_ACTION_ROW (adw_action_row_new ());
  adw_preferences_row_set_title (ADW_PREFERENCES_ROW (midnight_row), "Day Start (Midnight Offset)");
  GtkWidget *midnight_spin = gtk_spin_button_new (gtk_adjustment_new (0, 0, 23, 1, 1, 0), 1, 0);
  gtk_widget_set_valign (midnight_spin, GTK_ALIGN_CENTER);
  adw_action_row_add_suffix (midnight_row, midnight_spin);
  adw_preferences_group_add (group, GTK_WIDGET (midnight_row));
  g_settings_bind (settings, "midnight-offset", midnight_spin, "value", G_SETTINGS_BIND_DEFAULT);

  gtk_widget_show (dialog);
  g_object_unref (settings);
}

static void
on_activate_action (GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
  (void)action; (void)parameter;
  GtkApplication *app = GTK_APPLICATION (user_data);
  GtkWindow *window = gtk_application_get_active_window (app);
  if (window) {
    gtk_window_present (window);
  }
}

static void
on_about_action (GSimpleAction *action,
                 GVariant      *parameter,
                 gpointer       user_data)
{
  (void)action; (void)parameter;
  GtkApplication *app = GTK_APPLICATION (user_data);
  GtkWindow *parent = gtk_application_get_active_window (app);

  gtk_show_about_dialog (parent,
                         "program-name", "GTimer",
                         "logo-icon-name", "us.k5n.GTimer",
                         "version", VERSION,
                         "copyright", "© 1998-2026 Craig Knudsen",
                         "authors", (const char *[]) {"Craig Knudsen", NULL},
                         "license-type", GTK_LICENSE_GPL_2_0,
                         "website", "http://www.k5n.us/gtimer.php",
                         "website-label", "Website: k5n.us/gtimer",
                         "comments", "3.0.0 (2026-02-10)\n"
                                     "• Complete rewrite with GTK 4 and Libadwaita\n"
                                     "• Modernized UI with Search, Secondary Toolbar and Toast notifications\n"
                                     "• Improved data integrity with Auto-save and Midnight Rollover\n"
                                     "• Enhanced reporting with HTML support and search\n"
                                     "• Robust idle detection for Wayland and X11",
                         NULL);
}

static void
on_save_action (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
  (void)action;
  (void)parameter;
  (void)user_data;
  
  g_printerr ("DEBUG: Data saved.\n");
}

static void
on_quit_action (GSimpleAction *action,
                GVariant      *parameter,
                gpointer       user_data)
{
  (void)action;
  (void)parameter;
  GtkApplication *app = GTK_APPLICATION (user_data);
  GTimerApp *gtimer_app = g_object_get_data (G_OBJECT (app), "gtimer-app");
  
  // Stop all timers
  GListModel *model = gtimer_task_list_model_get_model (gtimer_app->task_list_model);
  guint n_items = g_list_model_get_n_items (model);
  for (guint i = 0; i < n_items; i++) {
    GTimerTask *task = GTIMER_TASK (g_list_model_get_item (model, i));
    if (gtimer_task_is_timing (task)) {
      gtimer_db_manager_stop_task_timing (gtimer_app->db_manager, gtimer_task_get_id (task));
    }
    g_object_unref (task);
  }
  
  g_application_quit (G_APPLICATION (app));
}

int
main (int argc, char **argv)
{
  AdwApplication *app;
  GTimerApp gtimer_app = {0};
  GError *error = NULL;
  int status;

  g_set_prgname ("us.k5n.GTimer");
  g_set_application_name ("GTimer");

  app = adw_application_new ("us.k5n.GTimer", G_APPLICATION_FLAGS_NONE);

  const GActionEntry app_entries[] = {
    { .name = "about", .activate = on_about_action },
    { .name = "report", .activate = on_report_action },
    { .name = "save", .activate = on_save_action },
    { .name = "quit", .activate = on_quit_action },
    { .name = "preferences", .activate = on_preferences_action },
    { .name = "activate", .activate = on_activate_action },
  };
  g_action_map_add_action_entries (G_ACTION_MAP (app), app_entries, G_N_ELEMENTS (app_entries), app);
  
  // Store gtimer_app for callbacks
  g_object_set_data (G_OBJECT (app), "gtimer-app", &gtimer_app);

  // Initialize DB in XDG data dir
  const char *data_dir = g_get_user_data_dir ();
  char *gtimer_dir = g_build_filename (data_dir, "gtimer", NULL);
  g_mkdir_with_parents (gtimer_dir, 0755);
  char *db_path = g_build_filename (gtimer_dir, "gtimer.db", NULL);
  g_print ("DEBUG: Using database at: %s\n", db_path);

  gtimer_app.db_manager = gtimer_db_manager_new (db_path, &error);
  if (!gtimer_app.db_manager) {
    g_printerr ("Failed to initialize database: %s\n", error->message);
    return 1;
  }

  gtimer_app.task_list_model = gtimer_task_list_model_new (gtimer_app.db_manager);
  gtimer_task_list_model_refresh (gtimer_app.task_list_model);

  gtimer_app.timer_service = gtimer_timer_service_new (gtimer_app.db_manager);
  
  // Set up idle monitoring
  gtimer_app.idle_monitor = gtimer_idle_monitor_new ();
  if (gtimer_idle_monitor_is_available (gtimer_app.idle_monitor)) {
    gtimer_timer_service_set_idle_monitor (gtimer_app.timer_service, gtimer_app.idle_monitor);
    g_print ("Idle monitoring enabled\n");
  } else {
    g_print ("Idle monitoring not available (not running under GNOME/Mutter)\n");
  }

  g_signal_connect (app, "activate", G_CALLBACK (activate), &gtimer_app);

  // Register Accelerators
  struct { const char *action; const char *accels[2]; } accels[] = {
    { "app.quit", { "<Control>q", NULL } },
    { "app.save", { "<Control>s", NULL } },
    { "app.report", { "<Control>r", NULL } },
    { "win.start-stop", { "<Alt>s", NULL } },
    { "win.stop-all", { "<Alt>t", NULL } },
    { "win.new-task", { "<Control>n", NULL } },
    { "win.edit-task", { "<Control>e", NULL } },
    { "win.annotate", { "<Control><Shift>a", NULL } },
    { "win.hide-task", { "<Control><Shift>h", NULL } },
    { "win.unhide-tasks", { "<Control><Shift>u", NULL } },
    { "win.delete-task", { "<Control>Delete", NULL } },
    { "win.adjust-time(60)", { "<Control><Shift>i", NULL } },
    { "win.adjust-time(300)", { "<Control>i", NULL } },
    { "win.adjust-time(1800)", { "<Control><Alt>i", NULL } },
    { "win.adjust-time(-60)", { "<Control><Shift>d", NULL } },
    { "win.adjust-time(-300)", { "<Control>d", NULL } },
    { "win.adjust-time(-1800)", { "<Control><Alt>d", NULL } },
    { "win.set-zero", { "<Control><Alt>0", NULL } },
    { "win.cut-time", { "<Control>x", NULL } },
    { "win.copy-time", { "<Control>c", NULL } },
    { "win.paste-time", { "<Control>v", NULL } },
  };

  for (guint i = 0; i < G_N_ELEMENTS (accels); i++) {
    gtk_application_set_accels_for_action (GTK_APPLICATION (app), accels[i].action, accels[i].accels);
  }

  status = g_application_run (G_APPLICATION (app), argc, argv);

  g_object_unref (gtimer_app.timer_service);
  g_object_unref (gtimer_app.task_list_model);
  g_clear_object (&gtimer_app.idle_monitor);
  gtimer_db_manager_free (gtimer_app.db_manager);
  g_object_unref (app);
  g_free (db_path);
  g_free (gtimer_dir);

  return status;
}
