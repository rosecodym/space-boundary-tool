Space Boundary Tool (version 1)

The Space Boundary Tool (SBT) is a tool for automatically calculating whole-building energy performance simulation thermal space boundaries for Industry Foundation Classes (IFC) building models, and using these space boundaries to automatically generate EnergyPlus Input Definition Files (IDFs).

If all you're looking for is the binary, it can be downloaded from https://gaia.lbl.gov/interoperability/SBT/. Read on for build instructions.

SBT requires Microsoft Visual Studio 2010 to build, and has three dependencies you'll need to acquire on your own:

Boost 1.47 - retrievable from http://www.boost.org/.
LEDA - retrievable from http://www.algorithmic-solutions.com/leda/index.htm. Only the free edition has been tested with SBT.
Express Data Manager (EDM) - A Jotne product. See http://www.epmtech.jotne.com/index.php?cat=41332. This component is not required to build the SBT Core (or its associated test project).

Once you have these, set up a Visual Studio Property Sheet, attach it to all configurations for the C++ and C++/CLI projects in the solution, and add the following three user-defined macros to it:
BoostDir - the root directory for your Boost installation.
LedaDir - the root directory for your LEDA installation.
EdmDir - the root directory for your EDM installation.

Refer to http://msdn.microsoft.com/en-us/library/vstudio/669zx6zc(v=vs.100).aspx for instructions on how to do this. Once this step is done, everything should build.

Direct questions to sbt-support@lbl.gov.