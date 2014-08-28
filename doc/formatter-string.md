#`String`-formatter
Maps log event to plain string by specified pattern with attributes support.

##Registration
When we register `string`-formatter with some sink `string_t` should be used. For example

```
repository_t::instance().configure<sink::syslog_t<level>, formatter::string_t>();
```

This code register `syslog`-sink and `string`-formatter pair. How to register another combinations of sinks and formatters check the ["Registration rules" article](registration-rules.md).

##Configuration

Typical configuration looks like

```
formatter_config_t formatter("string");
formatter["pattern"] = "[%(timestamp)s] [%(severity)s]: %(message)s";
```

The only parameter of the formatter is `pattern`. In this field we set the common view of log message. 

`pattern` consists of two types of entities. The first one is arbitrary symbols and the second type is placeholders. 

Common syntax for `pattern`

```
pattern  ::= ( literal | rph | oph | vph )+
literal  ::= char+
rph      ::= '%(' alnum+ ')s'
oph      ::= '%(' [char]* '[' alnum+ ']' [char]* ')?s'
vph      ::= '%(...' ( 'L' | 'E' | 'G' | 'T' | 'U' )+ ')s'
alnum    ::= ( alpha | num )+
alpha    ::= // any alphavite
num      ::= ‘0’ | ‘1’ | ‘2’ | ‘3’ | ‘4’ | ‘5’ | ‘6’ | ‘7’ | ‘8’ | ‘9’
```

`rph`, `oph` and `vph` are the placeholders of three types which are processed in different manner.

###Placeholders of type `rph`. Required placeholders

Syntax:
```
%(' alnum+ ')s
```

For example these are well known for us placeholder `%(timestamp)s`, `%(severity)s` and `%(message)s`.

This type of placeholder requires the same named attributes of log message. For macro `ВН_LOG` `severity` and `message` are the second and third parameter correspondingly. The `timestamp` is generated inside the macro.

```
BH_LOG(log, severity, "message");
```

Logger doesn't rise an exception on missing attributes while `BH_LOG` call, it send message to `stdout`.


###Placeholders of type 'oph'. Optional placeholders

Syntax:
```
%(' [char]* '[' alnum+ ']' [char]* ')?s
```

This placeholder doesn't require attribute in log message. If log message doesn't containe nesessary attribute all of the content inside braces will be skipped.

Let's consider the next pattern

```
formatter["pattern"] = "outer_symbols-%(inner_symbols<[optional_attr]>inner_symbols)?s-outer_symbols";
```

Calling of

```
BH_LOG(log, 0, "");
```
will produce the next output string

```
outer_symbols--outer_symbols
```

Calling of
```
BH_LOG(log, 0, "")("optional_attr","attr_value");
```

will produce the next output string

```
outer_symbols-inner_symbols<attr_value>inner_symbols-outer_symbols
```

###Placeholders of type `vph`. Variadic placeholders

Syntax:

```
'%(...' ( 'L' | 'E' | 'G' | 'T' | 'U' )+ ')s'
```

'L', 'E', 'G', 'T', 'U' are the types of attritubutes.

Attributes `"timestamp"`, `"severity"` and `"message"` have ??????-type. All other attributes have 'L'-type by default. How to set the type of attribute you can read in the ["Passing of attributes to the macro"](passing-attributes.md) article.

This placeholder get all of the attributes of log message of specified type and insert them into the final string in the next manner

```
?????????????????????????
```

##Example
In development

[Back to content](contents.md)