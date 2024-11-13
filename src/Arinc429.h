#pragma once

void xPlane12UDPtoArinc429(float source[], std::vector<bool> options, std::vector<uint32_t>& output);
void floatDataToArincProto(int optionId, float dataRow[], std::vector<uint32_t>& output);
uint32_t encodeArincBinary(uint8_t label, bool bit29, float value, uint8_t sigBits, float maxRange);
uint32_t encodeArincBinary(uint8_t label, bool bit29, float value, uint8_t sigBits, float maxRange, uint8_t SDI);