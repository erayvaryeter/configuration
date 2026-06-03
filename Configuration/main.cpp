#include <iostream>

#include "IConfiguration.h"

std::shared_ptr<IConfiguration> CreateRiasXmlFile() {
	auto riasXmlFile = ConfigurationFile::createFile(ConfigType::XML, "SUMER");
	if (riasXmlFile) {
		auto sumerRoot = riasXmlFile->getSubNode("SUMER");
		if (sumerRoot) {
			auto riasGroup = sumerRoot->createGroup("RIAS");
			if (riasGroup) {
				// haberlesme node
				auto riasHaberlesmeGroup = riasGroup->createGroup("HABERLESME");
				if (riasHaberlesmeGroup) {
					riasHaberlesmeGroup->appendNode("IP", "100.100.100.100");
					riasHaberlesmeGroup->appendNode<uint16_t>("PORT", 1000);
				}
				// other nodes
				riasGroup->appendNode("IBIT_TIMEOUT", 10000);
				riasGroup->appendNode<double>("PERIOD", 10.000);
				riasGroup->appendNode("LOG_ACTIVE", true);
				// platform nodes
				auto platform1 = riasGroup->createGroup("PLATFORM");
				platform1->appendNode("ID", 1);
				platform1->appendNode("LOCATION", "ANKARA");
				platform1->appendNode("LOCATION", "BURSA");
				auto platform2 = riasGroup->createGroup("PLATFORM");
				platform2->appendNode("ID", 10);
				platform2->appendNode("LOCATION", "KONYA");
				platform2->appendNode("LOCATION", "ERZURUM");
				auto platform3 = riasGroup->createGroup("PLATFORM");
				platform3->appendNode("ID", 100);
				platform3->appendNode("LOCATION", "MERSIN");
				platform3->appendNode("LOCATION", "ADANA");
			}
		}
		ConfigurationFile::saveToFile(riasXmlFile, "D:/config_test/Riax.xml");
	}
	return riasXmlFile;
}

std::shared_ptr<IConfiguration> CreateRfksJsonFile() {
	auto rfksJsonFile = ConfigurationFile::createFile(ConfigType::JSON, "SUMER");
	if (rfksJsonFile) {
		auto sumerRoot = rfksJsonFile->getSubNode("SUMER");
		if (sumerRoot) {
			auto rfksGroup = sumerRoot->createGroup("RFKS");
			if (rfksGroup) {
				// haberlesme node
				auto rfksHaberlesmeGroup = rfksGroup->createGroup("HABERLESME");
				if (rfksHaberlesmeGroup) {
					rfksHaberlesmeGroup->appendNode("IP", "200.200.200.200");
					rfksHaberlesmeGroup->appendNode("PORT", 2000);
				}
				// other nodes
				rfksGroup->appendNode("IBIT_TIMEOUT", 20000);
				rfksGroup->appendNode<float>("PERIOD", 20.00f);
				rfksGroup->appendNode("LOG_ACTIVE", false);
				// platform nodes
				auto platform1 = rfksGroup->createGroup("PLATFORM");
				platform1->appendNode("ID", 2);
				platform1->appendNode("LOCATION", "ISTANBUL");
				platform1->appendNode("LOCATION", "ZONGULDAK");
				auto platform2 = rfksGroup->createGroup("PLATFORM");
				platform2->appendNode("ID", 20);
				platform2->appendNode("LOCATION", "EDIRNE");
				platform2->appendNode("LOCATION", "TRABZON");
				auto platform3 = rfksGroup->createGroup("PLATFORM");
				platform3->appendNode("ID", 200);
				platform3->appendNode("LOCATION", "ESKISEHIR");
				platform3->appendNode("LOCATION", "VAN");
			}
		}
		ConfigurationFile::saveToFile(rfksJsonFile, "D:/config_test/Rfks.json");
	}
	return rfksJsonFile;
}

std::shared_ptr<IConfiguration> CreateCmdsYamlFile() {
	auto cmdsYamlFile = ConfigurationFile::createFile(ConfigType::YAML, "SUMER");
	if (cmdsYamlFile) {
		auto sumerRoot = cmdsYamlFile->getSubNode("SUMER");
		if (sumerRoot) {
			auto cmdsGroup = sumerRoot->createGroup("CMDS");
			if (cmdsGroup) {
				// haberlesme node
				auto rfksHaberlesmeGroup = cmdsGroup->createGroup("HABERLESME");
				if (rfksHaberlesmeGroup) {
					rfksHaberlesmeGroup->appendNode("IP", "300.300.300.300");
					rfksHaberlesmeGroup->appendNode("PORT", 3000);
				}
				// other nodes
				cmdsGroup->appendNode("IBIT_TIMEOUT", 30000);
				cmdsGroup->appendNode<float>("PERIOD", 30.00f);
				cmdsGroup->appendNode("LOG_ACTIVE", true);
				// platform nodes
				auto platform1 = cmdsGroup->createGroup("PLATFORM");
				platform1->appendNode("ID", 3);
				platform1->appendNode("LOCATION", "KOCAELI");
				platform1->appendNode("LOCATION", "IZMIR");
				auto platform2 = cmdsGroup->createGroup("PLATFORM");
				platform2->appendNode("ID", 30);
				platform2->appendNode("LOCATION", "GIRESUN");
				platform2->appendNode("LOCATION", "KASTAMONU");
				auto platform3 = cmdsGroup->createGroup("PLATFORM");
				platform3->appendNode("ID", 300);
				platform3->appendNode("LOCATION", "DENIZLI");
				platform3->appendNode("LOCATION", "MARDIN");
			}
		}
		ConfigurationFile::saveToFile(cmdsYamlFile, "D:/config_test/Cmds.yaml");
	}
	return cmdsYamlFile;
}

void updateRiasJsonFile() {
	auto riasJsonFile = ConfigurationFile::loadFromFile("D:/config_test/Rias.json", ConfigType::JSON);
	if (riasJsonFile) {
		auto riasGroup = riasJsonFile->getSubNode("SUMER.RIAS");
		if (riasGroup) {
			// ip and port
			std::string ip;
			int port;
			riasGroup->getWithDefaultValue<std::string>("HABERLESME.IP", ip, "0.0.0.0");
			riasGroup->getWithDefaultValue("HABERLESME.PORT", port, 0);
			std::cout << "Ip: " << ip << " - port: " << port << std::endl;
			riasGroup->setValue<std::string>("HABERLESME.IP", "150.150.150.150");
			riasGroup->setValue<int>("HABERLESME.PORT", 1500);
			// other values
			uint32_t ibitTimeout;
			bool logActive;
			float period;
			riasGroup->getWithDefaultValue<uint32_t>("IBIT_TIMEOUT", ibitTimeout, 0);
			riasGroup->getWithDefaultValue<bool>("LOG_ACTIVE", logActive, false);
			riasGroup->getWithDefaultValue<float>("PERIOD", period, 0.0f);
			std::cout << "Ibit timeout: " << ibitTimeout << " - log active: " << logActive << " - period: " << period << std::endl;
			riasGroup->setValue<uint32_t>("IBIT_TIMEOUT", 15000);
			riasGroup->setValue<bool>("LOG_ACTIVE", true);
			riasGroup->setValue<float>("PERIOD", 15.0f);
			// platform values
			auto platformNodes = riasGroup->getSubNodes("PLATFORM");
			for (auto& platform : platformNodes) {
				auto idNode = platform->getSubNode("ID");
				auto locations = platform->getSubNodes("LOCATION");
				if (idNode) {
					uint16_t id;
					idNode->getWithDefaultValue<uint16_t>(".", id, 0);
					std::cout << "Platform ID: " << id << std::endl;
					idNode->setValue<uint32_t>(".", 15);
				}
				for (auto& location : locations) {
					std::string locationStr;
					location->getWithDefaultValue<std::string>(".", locationStr, "");
					std::cout << "Platform Location: " << locationStr << std::endl;
					location->setValue<std::string>(".", "LOCATION_15");
				}
			}
		}
		ConfigurationFile::saveToFile(riasJsonFile, "D:/config_test/RiasUpdated.json");
	}
	std::cout << std::endl;
}

void updateRfksYamlFile() {
	auto rfksYamlFile = ConfigurationFile::loadFromFile("D:/config_test/Rfks.yaml", ConfigType::YAML);
	if (rfksYamlFile) {
		auto rfksGroup = rfksYamlFile->getSubNode("SUMER.RFKS");
		if (rfksGroup) {
			// ip and port
			std::string ip;
			int port;
			rfksGroup->getWithDefaultValue<std::string>("HABERLESME.IP", ip, "0.0.0.0");
			rfksGroup->getWithDefaultValue("HABERLESME.PORT", port, 0);
			std::cout << "Ip: " << ip << " - port: " << port << std::endl;
			rfksGroup->setValue<std::string>("HABERLESME.IP", "250.250.250.250");
			rfksGroup->setValue<int>("HABERLESME.PORT", 2500);
			// other values
			uint32_t ibitTimeout;
			bool logActive;
			float period;
			rfksGroup->getWithDefaultValue<uint32_t>("IBIT_TIMEOUT", ibitTimeout, 0);
			rfksGroup->getWithDefaultValue<bool>("LOG_ACTIVE", logActive, false);
			rfksGroup->getWithDefaultValue<float>("PERIOD", period, 0.0f);
			std::cout << "Ibit timeout: " << ibitTimeout << " - log active: " << logActive << " - period: " << period << std::endl;
			rfksGroup->setValue<uint32_t>("IBIT_TIMEOUT", 25000);
			rfksGroup->setValue<bool>("LOG_ACTIVE", true);
			rfksGroup->setValue<float>("PERIOD", 25.0f);
			// platform values
			auto platformNodes = rfksGroup->getSubNodes("PLATFORM");
			for (auto& platform : platformNodes) {
				auto idNode = platform->getSubNode("ID");
				auto locations = platform->getSubNodes("LOCATION");
				if (idNode) {
					uint16_t id;
					idNode->getWithDefaultValue<uint16_t>("", id, 0);
					std::cout << "Platform ID: " << id << std::endl;
					idNode->setValue<uint32_t>("", 25);
				}
				for (auto& location : locations) {
					std::string locationStr;
					location->getWithDefaultValue<std::string>("", locationStr, "");
					std::cout << "Platform Location: " << locationStr << std::endl;
					location->setValue<std::string>("", "LOCATION_25");
				}
			}
		}
		ConfigurationFile::saveToFile(rfksYamlFile, "D:/config_test/RfksUpdated.yaml");
	}
	std::cout << std::endl;
}

void updateCmdsXmlFile() {
	auto cmdsXmlFile = ConfigurationFile::loadFromFile("D:/config_test/Cmds.xml", ConfigType::XML);
	if (cmdsXmlFile) {
		auto cmdsGroup = cmdsXmlFile->getSubNode("SUMER.CMDS");
		if (cmdsGroup) {
			// ip and port
			std::string ip;
			int port;
			cmdsGroup->getWithDefaultValue<std::string>("HABERLESME.IP", ip, "0.0.0.0");
			cmdsGroup->getWithDefaultValue("HABERLESME.PORT", port, 0);
			std::cout << "Ip: " << ip << " - port: " << port << std::endl;
			cmdsGroup->setValue<std::string>("HABERLESME.IP", "350.350.350.350");
			cmdsGroup->setValue<int>("HABERLESME.PORT", 3500);
			// other values
			uint32_t ibitTimeout;
			bool logActive;
			float period;
			cmdsGroup->getWithDefaultValue<uint32_t>("IBIT_TIMEOUT", ibitTimeout, 0);
			cmdsGroup->getWithDefaultValue<bool>("LOG_ACTIVE", logActive, false);
			cmdsGroup->getWithDefaultValue<float>("PERIOD", period, 0.0f);
			std::cout << "Ibit timeout: " << ibitTimeout << " - log active: " << logActive << " - period: " << period << std::endl;
			cmdsGroup->setValue<uint32_t>("IBIT_TIMEOUT", 35000);
			cmdsGroup->setValue<bool>("LOG_ACTIVE", true);
			cmdsGroup->setValue<float>("PERIOD", 35.0f);
			// platform values
			auto platformNodes = cmdsGroup->getSubNodes("PLATFORM");
			for (auto& platform : platformNodes) {
				auto idNode = platform->getSubNode("ID");
				auto locations = platform->getSubNodes("LOCATION");
				if (idNode) {
					uint16_t id;
					idNode->getWithDefaultValue<uint16_t>("", id, 0);
					std::cout << "Platform ID: " << id << std::endl;
					idNode->setValue<uint32_t>("", 35);
				}
				for (auto& location : locations) {
					std::string locationStr;
					location->getWithDefaultValue<std::string>("", locationStr, "");
					std::cout << "Platform Location: " << locationStr << std::endl;
					location->setValue<std::string>("", "LOCATION_35");
				}
			}
		}
		ConfigurationFile::saveToFile(cmdsXmlFile, "D:/config_test/CmdsUpdated.xml");
	}
}

void appendAllNodesAndConvertSave(std::shared_ptr<IConfiguration>& riasXmlFile,
	std::shared_ptr<IConfiguration>& rfksJsonFile,
	std::shared_ptr<IConfiguration>& cmdsYamlFile) 
{
	// get related nodes
	auto xmlSumerNode = riasXmlFile->getSubNode("SUMER");
	auto jsonRfksNode = rfksJsonFile->getSubNode("SUMER.RFKS");
	auto yamlCmdsNode = cmdsYamlFile->getSubNode("SUMER.CMDS");
	// append rfks and cmds nodes to sumer node of rias xml file
	xmlSumerNode->appendSubNode("RFKS", jsonRfksNode);
	xmlSumerNode->appendSubNode("CMDS", yamlCmdsNode);
	// save the appended rias xml file
	ConfigurationFile::saveToFile(riasXmlFile, "D:/config_test/AllSystems.xml");
	// convert and save to json and yaml
	ConfigurationFile::convertAndSave(riasXmlFile, ConfigType::JSON, "D:/config_test/AllSystems.json");
	ConfigurationFile::convertAndSave(riasXmlFile, ConfigType::YAML, "D:/config_test/AllSystems.yaml");
}

int main() {

	// create rias xml file
	auto riasXmlFile = CreateRiasXmlFile();

	// create rfks json file
	auto rfksJsonFile = CreateRfksJsonFile();

	// create cmds yaml file
	auto cmdsYamlFile = CreateCmdsYamlFile();

	// convert rias xml to rias json file and rias yaml file
	ConfigurationFile::convertAndSave(riasXmlFile, ConfigType::JSON, "D:/config_test/Rias.json");
	ConfigurationFile::convertAndSave(riasXmlFile, ConfigType::YAML, "D:/config_test/Rias.yaml");
	
	// convert rfks json to rfks xml file and rfks yaml file
	ConfigurationFile::convertAndSave(rfksJsonFile, ConfigType::XML, "D:/config_test/Rfks.xml");
	ConfigurationFile::convertAndSave(rfksJsonFile, ConfigType::YAML, "D:/config_test/Rfks.yaml");

	// convert cmds yaml to cmds xml and cmds json file
	ConfigurationFile::convertAndSave(cmdsYamlFile, ConfigType::XML, "D:/config_test/Cmds.xml");
	ConfigurationFile::convertAndSave(cmdsYamlFile, ConfigType::JSON, "D:/config_test/Cmds.json");
	
	// update rias json file
	updateRiasJsonFile();

	// update rfks yaml file
	updateRfksYamlFile();

	// update cmds xml file
	updateCmdsXmlFile();
	
	// append all nodes to each other & convert and save to all formats
	appendAllNodesAndConvertSave(riasXmlFile, rfksJsonFile, cmdsYamlFile);

	return 0;
}