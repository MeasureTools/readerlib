<!-- MarkdownTOC -->

- [Measurement Data Reading and Exporting Library](#measurement-data-reading-and-exporting-library)
    - [Getting Started](#getting-started)
        - [Prerequisites](#prerequisites)
    - [Supported file formats](#supported-file-formats)
        - [META](#meta)
        - [CSV](#csv)
        - [Grim](#grim)
        - [PSI/PSD](#psipsd)
        - [XML](#xml)
    - [How to read measurement data](#how-to-read-measurement-data)
    - [How to export measurement data](#how-to-export-measurement-data)
    - [Running the tests](#running-the-tests)
    - [License](#license)
    - [Acknowledgments](#acknowledgments)

<!-- /MarkdownTOC -->

# Measurement Data Reading and Exporting Library

This is an open source library to read and write different types of measurement data file formats.

[![Build Status](https://travis-ci.org/MeasureTools/readerlib.svg?branch=master)](https://travis-ci.org/MeasureTools/readerlib)

## Getting Started

The simplest way to include this library to your project is by adding it as a gitsubmodule to your project.

```bash
git submodule add git@github.com:MeasureTools/readerlib.git 3rdparty/readerlib
```

Then add *readerlib* or *readerlib_s* to your CMake *target_link_libraries* and then include the wished header in your source code.

For example, to include the reader for Keysight dlog files use
```cpp
#include <rlib/keysight/dlog_reader.h>
```

### Prerequisites

This library requires [```boost```](http://www.boost.org/doc/libs/) and was tested with version **>=1.65**. It may work with earlier versions.

## Supported file formats

| Type    | Exporter Support | Reader Support |
| ------- | ---------------- | -------------- |
| meta    | Yes              | No             |
| csv     | Yes              | Yes            |
| grim    | Yes              | Yes            |
| psi/psd | Yes              | Yes            |
| remote  | No               | Yes            |
| svg     | Yes              | No             |
| xml     | Yes              | Yes            |

### META

### CSV

Currently the first column of the **cvs** needs to be the time in seconds as a floating point number. Each other column defines a sensor. The first row in the **csv** needs to include the sensor names and the value types. Also the seperator between the columns currently needs to be a comma(**,**).

Example:

```csv
time,Sensor Name (Unit),Another_Name (Unit2)
0.1, 10.0, 22.0
0.2, 12.0, 23.0
```

### Grim

The .grim file format (while not containing any measurement data itself) allows to combine different measurement file formats and basic manipulation of the measurements.

Example for an .grim file:

```xml
<?xml version="1.0" encoding="UTF-8" ?>
<grim version="1.0">
  <sources samplerate="1000">   <!- Note: samplerate in **Hz** ->
    <src id="0" file="Abolute/Path/To/MeasurementFile.ext"/>
    </sources>
    <sensors>
        <sensor name="MySensorName" unit="MySensorUnit">
            <const value="1"/>
            <plus src="0" sensor="0"/>
            <minus src="0" sensor="0"/>
            <times src="0" sensor="0"/>
            <divide src="0" sensor="0"/>
        </sensor>
        <sensor name="MySensorName2" unit="MySensorUnit2">
            <const value="0"/>
            <plus src="0" sensor="0"/>
            <plus value="1"/>
            <minus value="1"/>
            <times value="1"/>
            <divide value="1"/>
        </sensor>
    </sensors>
</grim>
```

### PSI/PSD

The .psi/.psd file format is outputed by Hitex PowerScale Tools. Currently v1.0 of the format is supported.

Example for an .psi file with one STD probe. 
```xml
<?xml version="1.0" encoding="UTF-8" ?>
<PSI>
  <Version Value="1.0" />           <!- Note: PSI Version ->
  <Checksum Value="0x00000000" />   <!- Note: Checksum seems to be always 0 ->
  <Measurement>
    <SamplingRate Value="10" />   <!- Note: the sampling rate in **kHz** ->
    <SamplingCount Value="50" />  <!- Note: Number of all samples ->
  </Measurement>
  <DataStream Count="1">            <!- Note: Count is number of probes (two values [U/I] per probe) ->
    <DataStream 
                    Id="1"            <!- Note: The Id marks the position of the data for this probe in a sample->
                    ProbeID="1"       <!- Note: Port on the PowerScale HW used by the probe ->
                    ProbeKind="2"     <!- Note: Type or probe STD=2, ACM=1 ->
                    voltageMin="2.5"  <!- Note: Optional, min. voltage over all values of this probe ->
                    voltageMax="4.0"  <!- Note: Optional, max. voltage over all values of this probe ->
                    currentMin="0"    <!- Note: Optional, min. current over all values of this probe ->
                    currentMax="2.9"  <!- Note: Optional, max. current over all values of this probe ->
    />
  </DataStream>
  <PSD Count="1">                   <!- Note: Count is number of psd files, each psd file has the size of 1GiB ->
    <Version Value="1.0" />       <!- Note: PSD Version ->
    <PSDFile 
            Id="1"                    <!- Note: Id of the psd file. By default this is $filename_$id.psd ->
            Offset="0"                <!- Note: Number of the first sample in the psd file ->
            DataCount="50"            <!- Note: Number of samples in the psd file ->
            EventCount="0"            <!- Note: Number of events with event_occur bit set to 1 ->
    />
  </PSD>
</PSI>
```

While the .psi file contains all meta information, the .psd files contain all the sample and event information as following.
```cpp
struct {
    struct {
        struct {
            double current; //In little endian
            double voltage; //In little endian
            } probes[#probes];
        struct {
            uint16_t value : 15;
            uint16_t occur : 1;//HSB
            } probe_events[#probes];
        struct {
            uint16_t value : 15;
            uint16_t occur : 1;//HSB
            } global_event;
        } samples[#samples_in_the_psd_file];
} psd;
```

### XML

Example for an .xml file with measurement data supported by readerlib
```xml
<?xml version="1.0" encoding="UTF-8"?>
<output>
    <sensors>
        <sensor id="0" name="SensorName1" unit="SensorUnit1" />
        <sensor id="0" name="SensorName2" unit="SensorUnit2" />
        <sensor id="0" name="SensorName3" unit="SensorUnit3" />
    </sensors>
    <dataset>
        <data time="0">
            <value sensor="0" value="0.5" />
            <value sensor="1" value="1.5" />
            <value sensor="2" value="1.5" />
        </data>
        <data time="0.2">
            <value sensor="0" value="0.5" />
            <value sensor="1" value="1.5" />
            <value sensor="2" value="1.5" />
        </data>
    </dataset>
    <events>
        <!- origin="-1" == global otherwise the sensor id->
        <event level="1" time="3" origin="-1"> 
            <message>MSG:TEST_GLOBAL_EVENT</message>
            <data>155</data>
        </event>
        <event level="2" time="4" origin="2">
            <message>MSG:TEST_SENSOR_2_EVENT</message>
            <data>5</data>
        </event>
        <event level="0" time="9" origin="0">
            <message>MSG:TEST_SENSOR_0_EVENT</message>
            <data>664</data>
        </event>
        <event level="3" time="9" origin="1">
            <message>MSG:TEST_SENSOR_1_EVENT</message>
            <data>2</data>
        </event>
    </events>
</output>
```

## How to read measurement data

```cpp
#include <rlib/keysight/dlog_reader.h>
#include <iostream>
#include <cstdlib>

int main(int argc, char* argv[]) {
    auto reader = rlib::keysight::dlog_reader("example.dlog");

    std::cout << "Filename:       " << reader.filename() << std::endl;

    std::cout << "----------- SENSORS -----------" << std::endl;
    auto sensors = reader.sensors();
    int i = 0;
    for(auto& sensor : sensors) {
        std::cout << "    " << "Sensor No." << i << std::endl;
        std::cout << "    " << "Unit:              " << sensor.unit << std::endl;
        std::cout << "    " << "Name:              " << sensor.name << std::endl;
        std::cout << "    " << "Date:              " << sensor.date << std::endl;
        std::cout << "    " << "Version:           " << sensor.version << std::endl;
        std::cout << "    " << "Sampling interval: " << sensor.sampling_interval << std::endl;
        std::cout << "    " << "Channel:           " << sensor.channel << std::endl;
        std::cout << std::endl;
        ++i;
    }

    std::cout << "----------- SAMPLES -----------" << std::endl;
    auto samples = reader.samples(0.0, reader.length());
    for(auto& sample : samples) {
        std::cout << "    " << "Time:" << sample.time << " seconds" << std::endl;
        int i = 0;
        for(auto value : sample.values) {
            std::cout << "    " << "Sensor No." << i << " Value: " << value << std::endl;
            ++i;
        }
        std::cout << std::endl;
    }

    std::cout << "----------- EVENTS -----------" << std::endl;
    auto events = reader.events(0.0, reader.length());
    for(auto& event : events) {
        switch(event.event_level) {
            case rlib::common::VERBOS:
                std::cout << "    " << "LEVEL:VERBOS" << std::endl;
            break;
            case rlib::common::DEBUG:
                std::cout << "    " << "LEVEL:DEBUG" << std::endl;
            break;
            case rlib::common::WARNING:
                std::cout << "    " << "LEVEL:WARNING" << std::endl;
            break;
            case rlib::common::ERROR:
                std::cout << "    " << "LEVEL:ERROR" << std::endl;
            break;
        }
        std::cout << "    " << "Time:" << event.time << " seconds" << std::endl;
        if (event.origin < 0) {
            std::cout << "    " << "Origin: GLOBAL" << std::endl;
        }
        else {
            std::cout << "    " << "Origin: Sensor No." << event.origin << std::endl;
        }
        std::cout << "    " << "Message:" << event.message << std::endl;
        std::cout << std::endl;
    }

    return EXIT_SUCCESS;
}
```

## How to export measurement data

```cpp
#include <rlib/keysight/dlog_reader.h>
#include <rlib/csv/csv_exporter.h>
#include <iostream>
#include <cstdlib>
#include <memory>

int main(int argc, char* argv[]) {
    auto shared_reader = std::make_shared<rlib::keysight::dlog_reader>("example.dlog");
    std::ofstream output("example.csv", std::ios::binary);

    rlib::csv::csv_exporter exporter(shared_reader);
    exporter.data_export(0.0, shared_reader->length(), output);

    return EXIT_SUCCESS;
}
```

## Running the tests

To run the tests do the following:

```bash
mkdir build
cd build
cmake ..
make
make test
```


## License

This project is licensed under a modified BSD 1-Clause License with an additional non-military use clause - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

* TU Dortmund [Embedded System Software Group](https://ess.cs.tu-dortmund.de/EN/Home/index.html) which made this release possible
