#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>

#define CONF_FILE_PATH "/etc/modprobe.d/tuxedo_keyboard.conf"
#define OPTIONS_FORMAT "options tuxedo-keyboard mode=%d brightness=%d color_left=0x%02X%02X%02X color_center=0x%02X%02X%02X color_right=0x%02X%02X%02X"

typedef struct TKeyboardOptions
{
	unsigned int mode;
	unsigned int brightness;
	GdkRGBA* color_left;
	GdkRGBA* color_center;
	GdkRGBA* color_right;
} KeyboardOptions;

KeyboardOptions* keyboard_options_new()
{
	KeyboardOptions* keyboard_options = calloc(sizeof(KeyboardOptions), 1);
	GdkRGBA* left = calloc(sizeof(GdkRGBA), 1);
	left->alpha = 1.0F;
	GdkRGBA* center = calloc(sizeof(GdkRGBA), 1);
	center->alpha = 1.0F;
	GdkRGBA* right = calloc(sizeof(GdkRGBA), 1);
	right->alpha = 1.0F;
	keyboard_options->color_left = left;
	keyboard_options->color_center = center;
	keyboard_options->color_right = right;
	return keyboard_options;
}

void free_keyboard_options(KeyboardOptions* keyboard_options)
{
	if (keyboard_options)
	{
		if (keyboard_options->color_left)
		{
			gdk_rgba_free(keyboard_options->color_left);
		}
		if (keyboard_options->color_center)
		{
			gdk_rgba_free(keyboard_options->color_center);
		}
		if (keyboard_options->color_right)
		{
			gdk_rgba_free(keyboard_options->color_right);
		}
		free(keyboard_options);
	}
}

int keyboard_options_to_string(KeyboardOptions* keyboard_options, char* msg)
{
	return sprintf(msg,
		OPTIONS_FORMAT,
		keyboard_options->mode,
		keyboard_options->brightness,
		(unsigned int)(keyboard_options->color_left->red * 255),
		(unsigned int)(keyboard_options->color_left->green * 255),
		(unsigned int)(keyboard_options->color_left->blue * 255),
		(unsigned int)(keyboard_options->color_center->red * 255),
		(unsigned int)(keyboard_options->color_center->green * 255),
		(unsigned int)(keyboard_options->color_center->blue * 255),
		(unsigned int)(keyboard_options->color_right->red * 255),
		(unsigned int)(keyboard_options->color_right->green * 255),
		(unsigned int)(keyboard_options->color_right->blue * 255)
	);
}

int read_keyboard_options(KeyboardOptions* keyboard_options)
{
	int res = 0;
	if (g_file_test(CONF_FILE_PATH, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))
	{
		GFile* file = g_file_new_for_path(CONF_FILE_PATH);
		GFileInputStream* stream = g_file_read(file, NULL, NULL);
		char buf[1024] = { 0 };
		gsize bytes_read;
		gboolean read = g_input_stream_read_all(G_INPUT_STREAM(stream), buf, 1024,
			&bytes_read, NULL, NULL);
		if (read)
		{
			int lr, lg, lb, cr, cg, cb, rr, rg, rb;
			res = sscanf(
				buf,
				OPTIONS_FORMAT,
				&(keyboard_options->mode),
				&(keyboard_options->brightness),
				&lr,
				&lg,
				&lb,
				&cr,
				&cg,
				&cb,
				&rr,
				&rg,
				&rb
			);
			keyboard_options->color_left->red = (float)lr / 255.0F;
			keyboard_options->color_left->green = (float)lg / 255.0F;
			keyboard_options->color_left->blue = (float)lb / 255.0F;
			keyboard_options->color_center->red = (float)cr / 255.0F;
			keyboard_options->color_center->green = (float)cg / 255.0F;
			keyboard_options->color_center->blue = (float)cb / 255.0F;
			keyboard_options->color_right->red = (float)rr / 255.0F;
			keyboard_options->color_right->green = (float)rg / 255.0F;
			keyboard_options->color_right->blue = (float)rb / 255.0F;
		}
		g_input_stream_close(G_INPUT_STREAM(stream), NULL, NULL);
	}
	else
	{
		g_printerr("Read conf file %s failed", CONF_FILE_PATH);
	}
	return res;
}

gboolean save_keyboard_options(KeyboardOptions* keyboard_options)
{
	char msg[1024];
	keyboard_options_to_string(keyboard_options, msg);
	printf("%s\n", msg);
	return g_file_set_contents_full(CONF_FILE_PATH, msg, (gssize)strlen(msg),
		G_FILE_SET_CONTENTS_CONSISTENT | G_FILE_SET_CONTENTS_ONLY_EXISTING,
		0644,
		NULL);
}

void brightness_label_change_with_scale(GtkRange* scale, gpointer data)
{
	GtkLabel* label = (GtkLabel*)data;
	char msg[32];
	double brightness = gtk_range_get_value(scale);
	sprintf(msg, "BRIGHTNESS: %d", (int)brightness);
	gtk_label_set_text(label, msg);
}

void brightness_change_with_scale(GtkRange* scale, gpointer data)
{
	KeyboardOptions* keyboard_options = (KeyboardOptions*)data;
	double brightness = gtk_range_get_value(scale);
	keyboard_options->brightness = (unsigned int)brightness;
}

void color_set(GtkColorButton* button, gpointer data)
{
	gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(button), (GdkRGBA*)data);
}

void mode_selected(GtkComboBoxText* combo_box_text, gpointer data)
{
	KeyboardOptions* keyboard_options = (KeyboardOptions*)data;
	int id = gtk_combo_box_get_active(GTK_COMBO_BOX(combo_box_text));
	keyboard_options->mode = id;
}

void on_confirm_button_clicked(GtkButton* ignored, gpointer data)
{
	KeyboardOptions* keyboard_options = (KeyboardOptions*)data;
	if (save_keyboard_options(keyboard_options))
	{
		g_print("Successful saved config\n");
	}
	else
	{
		g_printerr("Failed save config\n");
	}
}

int app_activate(GApplication* app, gpointer* user_data)
{

	KeyboardOptions* keyboard_options = (KeyboardOptions*)user_data;
#ifdef GTK_VERSION_4
	GtkWidget* window = gtk_window_new();
#else
	GtkWidget* window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
#endif
	gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));
	gtk_window_set_default_size(GTK_WINDOW(window), 640, 480);
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
	gtk_window_set_title(GTK_WINDOW(window), "Manage Keyboard");

	GtkWidget* table = gtk_grid_new();
	gtk_widget_set_margin_start(table, 20);
	gtk_widget_set_margin_end(table, 20);
	gtk_widget_set_margin_top(table, 20);
	gtk_widget_set_margin_bottom(table, 20);
	gtk_grid_set_row_spacing(GTK_GRID(table), 10);
	gtk_grid_set_column_spacing(GTK_GRID(table), 30);

#ifdef GTK_VERSION_4
	gtk_window_set_child(GTK_WINDOW(window), table);
#else
	gtk_container_add(GTK_CONTAINER(window), table);
#endif

	// Brightness
	char brightness_label_text[32];
	sprintf(brightness_label_text, "BRIGHTNESS: %d", keyboard_options->brightness);
	GtkWidget* brightness_label = gtk_label_new(brightness_label_text);
	GtkWidget* brightness_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 255.0, 1.0);
	gtk_range_set_value(GTK_RANGE(brightness_scale), keyboard_options->brightness);
	g_signal_connect(G_OBJECT(brightness_scale), "value_changed", G_CALLBACK(brightness_label_change_with_scale), (gpointer)brightness_label);
	g_signal_connect(G_OBJECT(brightness_scale), "value_changed", G_CALLBACK(brightness_change_with_scale), (gpointer)keyboard_options);
	gtk_grid_attach(GTK_GRID(table), brightness_label, 0, 0, 1, 1);
	gtk_grid_attach(GTK_GRID(table), brightness_scale, 1, 0, 10, 1);

	// Left color
	GtkWidget* left_color_label = gtk_label_new("LEFT COLOR");
	GtkWidget* left_color_chooser = gtk_color_button_new_with_rgba(keyboard_options->color_left);
	g_signal_connect(G_OBJECT(left_color_chooser), "color-set", G_CALLBACK(color_set), keyboard_options->color_left);
	gtk_grid_attach(GTK_GRID(table), left_color_label, 0, 1, 1, 1);
	gtk_grid_attach(GTK_GRID(table), left_color_chooser, 1, 1, 1, 1);

	// Center color
	GtkWidget* center_color_label = gtk_label_new("CENTER COLOR");
	GtkWidget* center_color_chooser = gtk_color_button_new_with_rgba(keyboard_options->color_center);
	g_signal_connect(G_OBJECT(center_color_chooser), "color-set", G_CALLBACK(color_set), keyboard_options
		->color_center);
	gtk_grid_attach(GTK_GRID(table), center_color_label, 0, 2, 1, 1);
	gtk_grid_attach(GTK_GRID(table), center_color_chooser, 1, 2, 1, 1);

	// Right color
	GtkWidget* right_color_label = gtk_label_new("RIGHT COLOR");
	GtkWidget* right_color_chooser = gtk_color_button_new_with_rgba(keyboard_options->color_right);
	g_signal_connect(G_OBJECT(right_color_chooser), "color-set", G_CALLBACK(color_set), keyboard_options->color_right);
	gtk_grid_attach(GTK_GRID(table), right_color_label, 0, 3, 1, 1);
	gtk_grid_attach(GTK_GRID(table), right_color_chooser, 1, 3, 1, 1);

	GtkWidget* mode_label = gtk_label_new("MODE");
	GtkWidget* mode_combo_box = gtk_combo_box_text_new();
	char* modes[8] = {
		"CUSTOM",
		"BREATHE",
		"CYCLE",
		"DANCE",
		"FLASH",
		"RANDOM_COLOR",
		"TEMPO",
		"WAVE"
	};
	for (int i = 0; i < 8; ++i)
	{
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(mode_combo_box), modes[i]);
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(mode_combo_box), (int)keyboard_options->mode);
	g_signal_connect(G_OBJECT(mode_combo_box), "changed", G_CALLBACK(mode_selected), (gpointer)keyboard_options);
	gtk_grid_attach(GTK_GRID(table), mode_label, 0, 5, 1, 1);
	gtk_grid_attach(GTK_GRID(table), mode_combo_box, 1, 5, 1, 1);


	// Confirm button
	GtkWidget* confirm_button = gtk_button_new_with_label("CONFIRM");
	g_signal_connect(G_OBJECT(confirm_button), "clicked", G_CALLBACK(on_confirm_button_clicked), (gpointer)keyboard_options);
	gtk_grid_attach(GTK_GRID(table), confirm_button, 0, 6, 1, 1);
#ifdef GTK_VERSION_4
	gtk_widget_show(GTK_WIDGET(window));
#else
	gtk_widget_show_all(GTK_WIDGET(window));
#endif
}

int main(int argc, char** argv)
{
	KeyboardOptions* keyboard_options = keyboard_options_new();
	read_keyboard_options(keyboard_options);
	GtkApplication* app;
	app = gtk_application_new("org.codebase.NH55KeyboardBackgroundManagement",
		G_APPLICATION_FLAGS_NONE);
	g_signal_connect(G_OBJECT(app), "activate", G_CALLBACK(app_activate), (gpointer)keyboard_options);
	int stat = g_application_run(G_APPLICATION(app), argc, argv);
	g_object_unref(app);
	free_keyboard_options(keyboard_options);
	return stat;
}
