using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;

namespace GUI.Converters
{
    [ValueConversion(typeof(bool), typeof(System.Windows.Visibility))]
    class FalseToVisibleConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            if (value is bool && (bool)value == false) { return System.Windows.Visibility.Visible; }
            else { return System.Windows.Visibility.Hidden; }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
