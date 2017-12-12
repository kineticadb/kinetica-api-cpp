// gpudbapiexe.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <avro/Compiler.hh>
#include "GPUdb.hpp"



int main(int argc, char* argv[])
{
	gpudb::GPUdb gpudb("http://gpudb.com/simba");
	std::string schemaString = "{ \"type\" : \"record\", \"name\" : \"TaxiTrack\", \"fields\" : [{ \"name\" : \"TRACKID\", \"type\" : \"string\" }, { \"name\" : \"TIMESTAMP\", \"type\" : \"long\" }, { \"name\" : \"x\", \"type\" : \"float\" }, { \"name\" : \"y\", \"type\" : \"float\" }, { \"name\" : \"SOURCELABEL\", \"type\" : \"string\" }]}";
	avro::ValidSchema schema = avro::compileJsonSchemaFromString(schemaString);
	std::vector<std::string> expressions;
	expressions.push_back("TRACKID = 'taxi_id_2396'");
	std::map<std::string, std::string> params;
	gpudb::BulkSelectResponse<avro::GenericDatum> response = gpudb.bulkSelect<avro::GenericDatum>(schema, "TaxiTrack", "", expressions, params);
	int ii = 0;
	for (std::vector<avro::GenericDatum>::const_iterator it = response.objects[0].begin(); it != response.objects[0].end(); ++it)
	{
		const avro::GenericRecord& record = (*it).value<avro::GenericRecord>();
		std::cout << record.field("TRACKID").value<std::string>() << std::endl;
		ii++;
	}
	std::cout << ii;
	return 0;
}


