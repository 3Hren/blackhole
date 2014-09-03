#Attribute mapping

If you want to change representation of attribute value in the final string (after the formatter) you can configure attribute mapping.

For example you can want to see levels of severity in your logs as srings like "DEBUG" instead of numbers '0' which you are use in the program. For this purpose you can use the followin code:

```
namespace {

//! Attribute mapping from its real values to human-readable string representation.
void map_severity(blackhole::aux::attachable_ostringstream& stream, level lvl) {
    static std::string LEVEL[] = {
        "DEBUG",
        "INFO",
        "WARNING",
        "ERROR"
    };

    auto value = static_cast<aux::underlying_type<level>::type>(lvl);
    if (value >= 0 && value < sizeof(LEVEL) / sizeof(LEVEL[0])) {
        stream << LEVEL[value];
    } else {
        stream << "UNKNOWN";
    }
}

} // namespace

void init(){
	
	//registration code
	
    mapping::value_t mapper;
    mapper.add<keyword::tag::severity_t<level>>(&map_severity);

    //further frontend configuration

}
```

Special case of attribute mapping is timestamp format changing. For this purpose Blackhole has a special syntax. You should specify format of timestamp as a string with placeholders like follows:

```
mapper.add<keyword::tag::timestamp_t>("%FT%T.%f");
```

**Placeholders**

|Placeholder|Description|
|-----------|-----------|
|F||
|T||
|f||


##Example
In development

[Back to contents](contenst.md)