## Time Object

This section describes an implementation of a standard Object defined in [OMA LwM2M Registry](https://technical.openmobilealliance.org/OMNA/LwM2M/LwM2MRegistry.html). As an example, we are going to implement [Time Object with ID 3333 in version 1.0.](https://raw.githubusercontent.com/OpenMobileAlliance/lwm2m-registry/prod/version_history/3333-1_0.xml) It is one of the simplest Objects defined in OMA LwM2M Registry, which allows to understand basics of operations on Objects.


This Objects contains definition of three resources. They are presented in the table below with their most important attributes.

| ID   |      Name     |  Operations |     Mandatory     |  Type  |  Description |
|----------|-------------|------|----------|-------------|------|
| 5506 |  Current Time | RW |  Mandatory | Time | Unix Time. A signed integer representing the number of seconds since Jan 1st, 1970 in the UTC time zone. |
| 5507 |  Fractional Time | RW |  Optional | Float | Fractional part of the time when sub-second precision is used (e.g., 0.23 for 230 ms).|
| 5750 |  Application Type | RW |  Optional | String | The application type of the sensor or actuator as a string depending on the use case.|


``ID`` - number used to identify the particular Resource. Different Objects may use the same Resource IDs for different purposes.

``Operations`` - RW indicates, that Resource is Readable and Writable.

``Mandatory`` - not all Resources defined for standard object must be implemented to be compliant with the specification. In this case, only the Current Time resource is mandatory.
