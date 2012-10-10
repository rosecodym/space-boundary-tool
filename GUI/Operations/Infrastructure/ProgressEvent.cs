using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace GUI.Operations
{
    class ProgressEvent
    {
        public enum ProgressEventType
        {
            Notification,
            Warning,
            Error
        }

        readonly string message;
        readonly ProgressEventType type;

        public string Message { get { return message; } }
        public ProgressEventType Type { get { return type; } }

        public ProgressEvent(string message, ProgressEventType type)
        {
            this.message = message;
            this.type = type;
        }
    }
}
