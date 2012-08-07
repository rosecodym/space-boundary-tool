using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;

namespace GUI.Converters
{
    [ValueConversion(typeof(BuildingInformation), typeof(string))]
    public class BuildingInformationToSbSummaryTitleConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            BuildingInformation info = value as BuildingInformation;
            if (info != null) { return "Space Boundary Summary (for " + info.IfcFilename + ")"; }
            else { return "Space boundary summary (no building loaded)"; }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new InvalidOperationException();
        }
    }
}
