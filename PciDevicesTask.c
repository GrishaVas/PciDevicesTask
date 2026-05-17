#include <sys/io.h>
#include <stdio.h>
#include <limits.h>
#include "pci.h"

void printPciDevices();
char* printPciDeviceDescription(unsigned int address, unsigned short vendorCode, unsigned short deviceCode, char* vendorName, char* deviceName);

char* getDeviceName(unsigned short deviceCode, unsigned short vendorCode);
char* getVendorName(unsigned short vendorCode);

unsigned short getDeviceCode(unsigned int data);
unsigned short getVendorCode(unsigned int data);

unsigned int getAddressForHeaderTypeData(unsigned int address);
unsigned char getHeaderType(unsigned int data);
void printHeaderType(unsigned int address);

unsigned int getAddressForExpansionROMBaseAddress(unsigned int address);
void printExpansionROMBaseAddress(unsigned int address);

int main()
{
    if (iopl(3))
    {
        printf("error");

        return 1;
    }

    //Функция вывода Pci устройств
    printPciDevices();

    return 0;
}

void printPciDevices()
{
    //установка бита enable
    unsigned int address = 0 |
        1 << 31;

    //Установка шага для адресов порта управления
    unsigned int busStep = 1 << 16;
    unsigned int deviceStep = 1 << 11;
    unsigned int functionStep = 1 << 8;

    for (size_t i = 0; i < 256; i++)
    {
        unsigned int busAddress = address;

        for (size_t j = 0; j < 32; j++)
        {
            unsigned int deviceAddress = busAddress;

            for (size_t k = 0; k < 8; k++)
            {
                //Установка адреса в регистр конфигурации
                outl(deviceAddress, 0x0CF8);

                //Получение значение регистра данных
                unsigned int data = inl(0x0CFC);

                if (data == 0 || data == UINT_MAX)
                {
                    break;
                }

                // printf("data: %x\n", data);
               
                //Получение кода устройства и кода производителя
                unsigned short deviceCode = getDeviceCode(data);
                unsigned short vendorCode = getVendorCode(data);

                //Расшифровка кода устройства и кода производителя
                char* deviceName = getDeviceName(deviceCode, vendorCode);
                char* vendorName = getVendorName(vendorCode);

                //Вывод описания
                printPciDeviceDescription(deviceAddress, vendorCode, deviceCode, vendorName, deviceName);

                //Вывод ExpansionROMBaseAddress
                printExpansionROMBaseAddress(deviceAddress);

                //Вывод HeaderType
                printHeaderType(deviceAddress);

                printf("\n");

                deviceAddress += functionStep;
            }


            busAddress += deviceStep;
        }


        address += busStep;
    }



}

void printExpansionROMBaseAddress(unsigned int address)
{
    //Установка адреса для получения ExpansionROMBaseAddress
    unsigned int addressForExpansionROMBaseAddress = getAddressForExpansionROMBaseAddress(address);

    outl(addressForExpansionROMBaseAddress, 0x0CF8);
    //Получение ExpansionROMBaseAddress
    unsigned int data = inl(0x0CFC);

    printf("Expansion ROM Base Address: %x\n", data);
}

unsigned int getAddressForExpansionROMBaseAddress(unsigned int address)
{
    //Смещение для ExpansionROMBaseAddress
    return address + 0x30;
}

void printHeaderType(unsigned int address)
{
    //Установка адреса для получения HeaderType
    unsigned int addressForHeaderType = getAddressForHeaderTypeData(address);

    //Установка адреса в регистр конфигурации
    outl(addressForHeaderType, 0x0CF8);
    //Получение HeaderType
    unsigned int data = inl(0x0CFC);

    unsigned char headerType = getHeaderType(data);

    // printf("address: %x\n", addressForHeaderType);
    // printf("data: %x\n", data);
    printf("Header type: %x", headerType);
}

unsigned int getAddressForHeaderTypeData(unsigned int address)
{
    //Смещение для HeaderType
    return address + 0x0C;
}

unsigned char getHeaderType(unsigned int data)
{
    return data >> 16;
}

unsigned short getVendorCode(unsigned int data)
{
    return data;
}

unsigned short getDeviceCode(unsigned int data)
{
    return data >> 16;
}


char* getVendorName(unsigned short vendorCode)
{
    for (size_t i = 0; i < PCI_VENTABLE_LEN; i++)
    {
        if (PciVenTable[i].VendorId == vendorCode)
        {
            return PciVenTable[i].VendorName;
        }

    }

    return "Not found.";
}

char* getDeviceName(unsigned short deviceCode, unsigned short vendorCode)
{

    for (size_t i = 0; i < PCI_DEVTABLE_LEN; i++)
    {
        if (PciDevTable[i].DeviceId == deviceCode && PciDevTable[i].VendorId == vendorCode)
        {
            return PciDevTable[i].DeviceName;
        }
    }

    return "Not found.";
}

char* printPciDeviceDescription(unsigned int address, unsigned short vendorCode, unsigned short deviceCode, char* vendorName, char* deviceName)
{
    printf("Address: %x\n", address);
    printf("Vendor code: %hx\n", vendorCode);
    printf("Device code: %hx\n", deviceCode);
    printf("Vendor name: %s\n", vendorName);
    printf("Device name: %s\n", deviceName);
}