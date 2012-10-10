using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Operations
{
    class Problem
    {
        public enum ProblemType
        {
            Warning,
            Error
        }

        readonly ProblemType type;
        readonly string message;

        public Problem(ProblemType type, string message)
        {
            this.type = type;
            this.message = message;
        }

        public ProblemType Type { get { return type; } }
        public string Message { get { return message; } }
    }
}
