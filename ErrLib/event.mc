; // event.mc 

; // This is the header section.


SeverityNames=(Success=0x0:STATUS_SEVERITY_SUCCESS
               Informational=0x1:STATUS_SEVERITY_INFORMATIONAL
               Warning=0x2:STATUS_SEVERITY_WARNING
               Error=0x3:STATUS_SEVERITY_ERROR
              )


FacilityNames=(System=0x0:FACILITY_SYSTEM
               Runtime=0x2:FACILITY_RUNTIME
               Stubs=0x3:FACILITY_STUBS
               Io=0x4:FACILITY_IO_ERROR_CODE
              )

LanguageNames=(English=0x409:MSG00409)


; // The following are the categories of events.

MessageIdTypedef=WORD

MessageId=0x1
SymbolicName=ERRLIB_CATEGORY
Language=English
Application Events
.

; // The following are the message definitions.

MessageIdTypedef=DWORD

MessageId=0x100
Severity=Error
Facility=Runtime
SymbolicName=MSG_ERROR
Language=English
%1
.

MessageId=0x101
Severity=Warning
Facility=Runtime
SymbolicName=MSG_WARNING
Language=English
%1
.

MessageId=0x102
Severity=Informational
Facility=Runtime
SymbolicName=MSG_INFORMATION
Language=English
%1
.
