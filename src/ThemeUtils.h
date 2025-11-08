#pragma once

#include <QWidget>
#include <QColor>

namespace ThemeUtils {

	/**
	 * @brief Sets the widget and all its child elements to dark or light mode.
	 *
	 * @param widget Pointer to the QWidget to apply the theme to.
	 * @param darkMode If true, applies dark mode; otherwise, applies light mode.
	 */
	void setWidgetDarkMode(QWidget* widget, bool darkMode);

} // namespace ThemeUtils
