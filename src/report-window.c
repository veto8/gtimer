#include "report-window.h"
#include <string.h>

struct _GTimerReportWindow
{
  AdwWindow parent_instance;
  GtkTextView *text_view;
};

G_DEFINE_TYPE (GTimerReportWindow, gtimer_report_window, ADW_TYPE_WINDOW)

static void
gtimer_report_window_class_init (GTimerReportWindowClass *klass)
{
  (void)klass;
}

static GFile *last_save_folder = NULL;

static void
on_save_response (GtkNativeDialog *native, int response_id, gpointer user_data)
{
  if (response_id == GTK_RESPONSE_ACCEPT) {
    GtkFileChooser *chooser = GTK_FILE_CHOOSER (native);
    GTimerReportWindow *self = GTIMER_REPORT_WINDOW (user_data);
    GFile *file = gtk_file_chooser_get_file (chooser);
    
    if (file) {
      // Save the folder for next time
      g_clear_object (&last_save_folder);
      last_save_folder = g_file_get_parent (file);

      GtkTextBuffer *buffer = gtk_text_view_get_buffer (self->text_view);
      GtkTextIter start, end;
      gtk_text_buffer_get_bounds (buffer, &start, &end);
      char *text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
      
      g_file_replace_contents (file, text, strlen (text), NULL, FALSE, G_FILE_CREATE_NONE, NULL, NULL, NULL);
      
      g_free (text);
      g_object_unref (file);
    }
  }
  g_object_unref (native);
}

static void
on_save_clicked (GtkButton *button, gpointer user_data)
{
  (void)button;
  GTimerReportWindow *self = GTIMER_REPORT_WINDOW (user_data);
  GtkFileChooserNative *native = gtk_file_chooser_native_new ("Save Report",
                                                              GTK_WINDOW (self),
                                                              GTK_FILE_CHOOSER_ACTION_SAVE,
                                                              "_Save",
                                                              "_Cancel");
  
  GDateTime *now = g_date_time_new_now_local ();
  char *date_slug = g_date_time_format (now, "%Y%m%d");
  char *default_filename = g_strdup_printf ("gtimer-report-%s.txt", date_slug);
  gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (native), default_filename);
  g_free (default_filename);
  g_free (date_slug);
  g_date_time_unref (now);

  if (last_save_folder) {
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (native), last_save_folder, NULL);
  }

  g_signal_connect (native, "response", G_CALLBACK (on_save_response), self);
  gtk_native_dialog_show (GTK_NATIVE_DIALOG (native));
}

static void
on_search_changed (GtkSearchEntry *entry, gpointer user_data)
{
  GTimerReportWindow *self = GTIMER_REPORT_WINDOW (user_data);
  const char *text = gtk_editable_get_text (GTK_EDITABLE (entry));
  GtkTextBuffer *buffer = gtk_text_view_get_buffer (self->text_view);
  GtkTextIter start, end;
  
  gtk_text_buffer_get_start_iter (buffer, &start);
  gtk_text_buffer_get_end_iter (buffer, &end);
  gtk_text_buffer_remove_tag_by_name (buffer, "found", &start, &end);

  if (text && strlen (text) > 0) {
    GtkTextIter match_start, match_end;
    while (gtk_text_iter_forward_search (&start, text, GTK_TEXT_SEARCH_CASE_INSENSITIVE, &match_start, &match_end, NULL)) {
      gtk_text_buffer_apply_tag_by_name (buffer, "found", &match_start, &match_end);
      start = match_end;
    }
  }
}

static void
begin_print (GtkPrintOperation *operation, GtkPrintContext *context, gpointer user_data)
{
  (void)operation; (void)context; (void)user_data;
  gtk_print_operation_set_n_pages (operation, 1);
}

static void
draw_page (GtkPrintOperation *operation, GtkPrintContext *context, int page_nr, gpointer user_data)
{
  (void)operation; (void)page_nr;
  GTimerReportWindow *self = GTIMER_REPORT_WINDOW (user_data);
  cairo_t *cr = gtk_print_context_get_cairo_context (context);
  double width = gtk_print_context_get_width (context);
  
  PangoLayout *layout = gtk_print_context_create_pango_layout (context);
  PangoFontDescription *desc = pango_font_description_from_string ("Monospace 10");
  pango_layout_set_font_description (layout, desc);
  pango_font_description_free (desc);

  GtkTextBuffer *buffer = gtk_text_view_get_buffer (self->text_view);
  GtkTextIter start, end;
  gtk_text_buffer_get_bounds (buffer, &start, &end);
  char *text = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
  
  pango_layout_set_text (layout, text, -1);
  pango_layout_set_width (layout, width * PANGO_SCALE);
  
  cairo_move_to (cr, 0, 0);
  pango_cairo_show_layout (cr, layout);
  
  g_free (text);
  g_object_unref (layout);
}

static void
on_print_clicked (GtkButton *button, gpointer user_data)
{
  (void)button;
  GTimerReportWindow *self = GTIMER_REPORT_WINDOW (user_data);
  GtkPrintOperation *print = gtk_print_operation_new ();
  
  g_signal_connect (print, "begin-print", G_CALLBACK (begin_print), self);
  g_signal_connect (print, "draw-page", G_CALLBACK (draw_page), self);
  
  gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW (self), NULL);
  g_object_unref (print);
}

static void
gtimer_report_window_init (GTimerReportWindow *self)
{
  GtkWidget *main_box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 0);
  adw_window_set_content (ADW_WINDOW (self), main_box);

  AdwHeaderBar *header_bar = ADW_HEADER_BAR (adw_header_bar_new ());
  gtk_box_append (GTK_BOX (main_box), GTK_WIDGET (header_bar));

  GtkWidget *search_bar = gtk_search_bar_new ();
  gtk_box_append (GTK_BOX (main_box), search_bar);
  
  GtkWidget *search_entry = gtk_search_entry_new ();
  gtk_search_bar_set_child (GTK_SEARCH_BAR (search_bar), search_entry);
  gtk_search_bar_set_key_capture_widget (GTK_SEARCH_BAR (search_bar), GTK_WIDGET (self));
  gtk_search_bar_set_search_mode (GTK_SEARCH_BAR (search_bar), TRUE);
  g_signal_connect (search_entry, "search-changed", G_CALLBACK (on_search_changed), self);

  GtkWidget *scrolled = gtk_scrolled_window_new ();
  gtk_widget_set_vexpand (scrolled, TRUE);
  gtk_box_append (GTK_BOX (main_box), scrolled);

  self->text_view = GTK_TEXT_VIEW (gtk_text_view_new ());
  gtk_text_view_set_editable (self->text_view, FALSE);
  gtk_text_view_set_monospace (self->text_view, TRUE);
  gtk_scrolled_window_set_child (GTK_SCROLLED_WINDOW (scrolled), GTK_WIDGET (self->text_view));

  GtkWidget *save_button = gtk_button_new_with_label ("Save");
  adw_header_bar_pack_start (header_bar, save_button);
  g_signal_connect (save_button, "clicked", G_CALLBACK (on_save_clicked), self);

  GtkWidget *print_button = gtk_button_new_with_label ("Print");
  adw_header_bar_pack_start (header_bar, print_button);
  g_signal_connect (print_button, "clicked", G_CALLBACK (on_print_clicked), self);

  gtk_window_set_default_size (GTK_WINDOW (self), 600, 600);
}

GTimerReportWindow *
gtimer_report_window_new (GtkWindow *parent, const char *title, const char *report_text)
{
  GTimerReportWindow *self = g_object_new (GTIMER_TYPE_REPORT_WINDOW, NULL);
  gtk_window_set_transient_for (GTK_WINDOW (self), parent);
  gtk_window_set_title (GTK_WINDOW (self), title);

  GtkTextBuffer *buffer = gtk_text_view_get_buffer (self->text_view);
  gtk_text_buffer_create_tag (buffer, "found", "background", "yellow", "foreground", "black", NULL);
  gtk_text_buffer_set_text (buffer, report_text, -1);

  return self;
}