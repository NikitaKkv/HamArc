#include <iostream>
#include <fstream>
#include <string>
#include <ios>
#include "../app/archiver.h"

int main(int argc, char* argv[]) {
	Archiver archiver(argc, argv);
	archiver.AddOptions();
	archiver.Run();
	return 0;
}
