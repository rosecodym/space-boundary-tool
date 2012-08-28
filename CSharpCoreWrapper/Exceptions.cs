using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Sbt
{
    namespace Exceptions
    {
        public class SbtException : Exception { }

        public class TooComplicatedException : SbtException { }
    }
}
