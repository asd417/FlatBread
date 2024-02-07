#include "entity_component_system.h"
#include "entity_components.h"
#include <fstream>

ECS::Coordinator* ECS::Coordinator::_globalCoordinator = nullptr;

ECS::Coordinator* ECS::Coordinator::GetCoordinator()
{
    if (_globalCoordinator == nullptr) _globalCoordinator = new ECS::Coordinator();
    return _globalCoordinator;
}

void ECS::Coordinator::DeleteCoordinator()
{
    if (_globalCoordinator != nullptr) delete _globalCoordinator;
}

void ECS::Coordinator::Serialize()
{
    std::ofstream fs("gameState.dat", std::ios::binary);

    for (auto it = componentManager->getComponentTypeIteratorBegin(); it != componentManager->getComponentTypeIteratorEnd(); it++) {
        auto compArray = componentManager->GetComponentArraySerializable(it->first);
        uint32_t stride = compArray->compSize/sizeof(SerializableComponent);
        uint32_t structNameLen = strlen(it->first);
        uint32_t compCount = compArray->size;
        fs.write(reinterpret_cast<const char*>(&structNameLen), sizeof(uint32_t));
        fs.write(reinterpret_cast<const char*>(it->first), sizeof(char) * (structNameLen+1));
        fs.write(reinterpret_cast<const char*>(&compCount), sizeof(uint32_t));
        
        //Remember that intel-based system uses little-endian
        for (uint32_t i = 0; i < compArray->size;i++) {
            compArray->componentArray[i * stride].Serialize(fs);
        }
    }
    const char* eof = "EOF";
    fs.write(eof, sizeof(char) * 3);
    fs.close();
}

void ECS::Coordinator::Deserialize()
{
    std::ifstream fs("gameState.dat", std::ios::binary);
    bool read = true;
    while (read) {
        uint32_t compNameSize;
        fs.read(reinterpret_cast<char*>(&compNameSize),sizeof(uint32_t));
        char compNameBuff[64]; //Assuming that No component will have name longer than (64-7) = 57 characters (because all comp name starts with 'struct '
        fs.read(reinterpret_cast<char*>(&compNameBuff), sizeof(char) * (compNameSize + 1));
        uint32_t count;
        fs.read(reinterpret_cast<char*>(&count), sizeof(uint32_t));
        std::cout << "read struct name: " << compNameBuff << " with count of " << count << "\n";

        auto compArray = componentManager->GetComponentArraySerializableByComp(reinterpret_cast<const char*>(&compNameBuff));
        uint32_t stride = compArray->compSize / sizeof(SerializableComponent);
        uint32_t structNameLen = strlen(compNameBuff);
        std::cout << "length of comp name: " << structNameLen << "\n";
        for (uint32_t i = 0; i < count; i++) {
            compArray->componentArray[i*stride].Deserialize(fs);
        }
        read = false;
    }
    fs.close();
}
