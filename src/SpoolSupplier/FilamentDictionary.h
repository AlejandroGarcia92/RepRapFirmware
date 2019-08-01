/*
 * FilamentDictionary.h
 *
 *  Created on: 4 jun. 2019
 *      Author: agarciamoreno
 */

#ifndef SRC_SPOOLSUPPLIER_FILAMENTDICTIONARY_H_
#define SRC_SPOOLSUPPLIER_FILAMENTDICTIONARY_H_

#include <cstdint>

// Result codes returned by temperature sensor drivers
enum class FilamentDictionary : uint32_t
{
	PLA = 3508469,//F5 88 35 00
	PVA = 77811945,//E9 50 A3 04
	PET_G = 944651,
	Nylon = 11545581,//ED 2B B0 00
	ABS = 1420656,
	TPU = 35468,
	defauld_filament = 0

};

const char* FilamentDictionaryString(FilamentDictionary name);
const float FilamentDictionaryTargetTemp(FilamentDictionary name);

#endif /* SRC_SPOOLSUPPLIER_FILAMENTDICTIONARY_H_ */
