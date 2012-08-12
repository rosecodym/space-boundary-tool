using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Data;

namespace GUI.Converters
{
    [ValueConversion(typeof(SbtBuildingInformation), typeof(string))]
    public class BuildingInformationToSbSummaryTitleConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            SbtBuildingInformation info = value as SbtBuildingInformation;
            if (info != null) { return "Space Boundary Summary (for " + info.IfcFilename + ")"; }
            else { return "Space boundary summary (no building loaded)"; }
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new InvalidOperationException();
        }
    }

    [ValueConversion(typeof(SbtBuildingInformation), typeof(string))]
    public class BuildingInformationTo2ndLevelPhysicalInternalCountString : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            SbtBuildingInformation info = value as SbtBuildingInformation;
            if (info != null)
            {
                SpaceBoundaryCollection sbs = info.SpaceBoundaries as SpaceBoundaryCollection;
                if (sbs != null) { return sbs.SecondLevelPhysicalInternalCount.ToString(); }
            }
            return "(no space boundaries loaded)";
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new InvalidOperationException();
        }
    }

    [ValueConversion(typeof(SbtBuildingInformation), typeof(string))]
    public class BuildingInformationTo2ndLevelPhysicalExternalCountString : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            SbtBuildingInformation info = value as SbtBuildingInformation;
            if (info != null)
            {
                SpaceBoundaryCollection sbs = info.SpaceBoundaries as SpaceBoundaryCollection;
                if (sbs != null) { return sbs.SecondLevelPhysicalExternalCount.ToString(); }
            }
            return "(no space boundaries loaded)";
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new InvalidOperationException();
        }
    }

    [ValueConversion(typeof(SbtBuildingInformation), typeof(string))]
    public class BuildingInformationTo3rdLevelCountString : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            SbtBuildingInformation info = value as SbtBuildingInformation;
            if (info != null)
            {
                SpaceBoundaryCollection sbs = info.SpaceBoundaries as SpaceBoundaryCollection;
                if (sbs != null) { return sbs.ThirdLevelCount.ToString(); }
            }
            return "(no space boundaries loaded)";
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new InvalidOperationException();
        }
    }

    [ValueConversion(typeof(SbtBuildingInformation), typeof(string))]
    public class BuildingInformationTo4thLevelCountString : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            SbtBuildingInformation info = value as SbtBuildingInformation;
            if (info != null)
            {
                SpaceBoundaryCollection sbs = info.SpaceBoundaries as SpaceBoundaryCollection;
                if (sbs != null) { return sbs.FourthLevelCount.ToString(); }
            }
            return "(no space boundaries loaded)";
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new InvalidOperationException();
        }
    }

    [ValueConversion(typeof(SbtBuildingInformation), typeof(string))]
    public class BuildingInformationToVirtualCountString : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            SbtBuildingInformation info = value as SbtBuildingInformation;
            if (info != null)
            {
                SpaceBoundaryCollection sbs = info.SpaceBoundaries as SpaceBoundaryCollection;
                if (sbs != null) { return sbs.VirtualCount.ToString(); }
            }
            return "(no space boundaries loaded)";
        }

        public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
        {
            throw new InvalidOperationException();
        }
    }
}
