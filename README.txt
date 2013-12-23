Space Boundary Tool (version 1)

The Space Boundary Tool (SBT) is a tool for automatically calculating whole-building energy performance simulation thermal space boundaries for Industry Foundation Classes (IFC) building models, and using these space boundaries to automatically generate EnergyPlus Input Definition Files (IDFs).

If all you're looking for is the binary, it can be downloaded from https://gaia.lbl.gov/interoperability/SBT/. Read on for build instructions.

SBT requires Microsoft Visual Studio 2010 to build, and has two dependencies you'll need to acquire on your own:

Boost 1.47 - retrievable from http://www.boost.org/. While SBT requires two Boost binaries, they are provided in /dependencies, so you just need to get the header-only libraries.
LEDA - retrievable from http://www.algorithmic-solutions.com/leda/index.htm. Only the free edition has been tested with SBT.

Once you have these installed, you'll need to configure their locations in any unmanaged projects you wish to build. This probably means these projects: Core, Core Tests, Ifc Adapter. The properties to set are in Configuration Properties > SBT Dependencies. After doing so, everything* should build.

Direct questions to sbt-support@lbl.gov.

*Note that the "Edm Wrapper" project will not be buildable unless you happen to have an EDM license. EDM is a Jotne product that we use for IFC interaction. The "Edm Wrapper" project cripples this software so that we can distribute it so that you can build SBT on your own. Everything else in SBT can be built, though, as a working Edm Wrapper binary (as well as its dependencies) is located in /dependencies. Issuing a "Build All" command for the solution will not attempt to build the Edm Wrapper. If you happen to have an EDM license of your own, you can build and tinker with the Edm Wrapper yourself by configuring the relevant properties in the SBT Dependencies section of Edm Wrapper's properties page.