#pragma once
#include <cassert>
#include <set>
#include <queue>
#include <array>
#include <bitset>
#include <memory>
#include <unordered_map>
#include <initializer_list>

#include "entity_components.h"

//https://austinmorlan.com/posts/entity_component_system/#demo

typedef uint32_t Entity;
typedef uint32_t ComponentType;

#define MAX_ENTITIES 5000
#define MAX_COMPONENTS 100

namespace ECS {
//using Signature = std::bitset<ECS::MAX_COMPONENTS>;
	typedef std::bitset<MAX_COMPONENTS> Signature;
	//constexpr int MAX_ENTITIES = 5000;
	//constexpr int MAX_COMPONENTS = 100;

	//base class of all systems that needs to iterate over the entities
	class EntitySystem {
	public:
		std::set<Entity> mEntities;
	};

	class IComponentArray {
	public:
		virtual ~IComponentArray() = default;
		virtual void EntityDestroyed(Entity entity) = 0;
	};

	template<typename T>
	class ComponentArray : public IComponentArray {
	public:
		ComponentArray(uint16_t entSize) { compSize = entSize; }
		T* Insert(Entity entity, T component) {
			assert(entityToIndexMap.find(entity) == entityToIndexMap.end() && "Entity already has component type");

			uint32_t newIndex = size;
			entityToIndexMap[entity] = newIndex;
			indexToEntityMap[newIndex] = entity;
			componentArray[newIndex] = component;
			size++;
			//std::cout << "TC ADDED pointer:" << &componentArray[newIndex] << "\n";
			return &componentArray[newIndex];
		}

		void Remove(Entity entity) {
			assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Entity does not own the component");

			uint32_t indexToRemove = entityToIndexMap[entity];
			uint32_t lastIndex = size - 1;
			// overwrite the entity to remove with the last element in the component array
			componentArray[indexToRemove] = componentArray[lastIndex];

			Entity moveEntity = indexToEntityMap[lastIndex];
			entityToIndexMap[moveEntity] = indexToRemove;
			indexToEntityMap[indexToRemove] = moveEntity;

			entityToIndexMap.erase(entity);
			indexToEntityMap.erase(lastIndex);

			size--;
		}

		T& Get(Entity entity)
		{
			assert(entityToIndexMap.find(entity) != entityToIndexMap.end() && "Retrieving non-existent component.");

			// Return a reference to the entity's component
			return componentArray[entityToIndexMap[entity]];
		}

		void EntityDestroyed(Entity entity) override
		{
			if (entityToIndexMap.find(entity) != entityToIndexMap.end())
			{
				// Remove the entity's component if it existed
				Remove(entity);
			}
		}

		uint32_t compSize = 0; //Byte size of the component
		uint32_t size = 0;
		std::array<T, MAX_ENTITIES> componentArray;
	private:

		// Map from an entity ID to an array index.
		std::unordered_map<Entity, size_t> entityToIndexMap;

		// Map from an array index to an entity ID.
		std::unordered_map<size_t, Entity> indexToEntityMap;

	};

	class ComponentManager {
	public:
		template<typename T>
		void RegisterComponent()
		{
			const char* typeName = typeid(T).name();

			assert(componentTypes.find(typeName) == componentTypes.end() && "Registering component type more than once.");

			// Add this component type to the component type map
			componentTypes.insert({ typeName, nextComponentType });

			// Create a ComponentArray pointer and add it to the component arrays map
			componentArrays.insert({ typeName, std::make_shared<ComponentArray<T>>(sizeof(T)) });

			// Increment the value so that the next component registered will be different
			++nextComponentType;
		}

		template<typename T>
		ComponentType GetComponentType()
		{
			const char* typeName = typeid(T).name();

			assert(componentTypes.find(typeName) != componentTypes.end() && "Component not registered before use.");

			// Return this component's type - used for creating signatures
			return componentTypes[typeName];
		}

		std::string GetComponentString(ComponentType ct) {
			for (auto& it : componentTypes) {
				if (it.second == ct) {
					return std::string(it.first);
				}
			}
			return "";
		}

		template<typename T>
		T* AddComponent(Entity entity, T component)
		{
			// Add a component to the array for an entity
			return GetComponentArray<T>()->Insert(entity, component);
		}

		template<typename T>
		void RemoveComponent(Entity entity)
		{
			// Remove a component from the array for an entity
			GetComponentArray<T>()->Remove(entity);
		}

		template<typename T>
		T& GetComponent(Entity entity)
		{
			// Get a reference to a component from the array for an entity
			return GetComponentArray<T>()->Get(entity);
		}

		void EntityDestroyed(Entity entity)
		{
			// Notify each component array that an entity has been destroyed
			// If it has a component for that entity, it will remove it
			for (auto const& pair : componentArrays)
			{
				auto const& component = pair.second;

				component->EntityDestroyed(entity);
			}
		}

		std::unordered_map<const char*, ComponentType>::iterator getComponentTypeIteratorBegin() {
			return componentTypes.begin();
		}

		std::unordered_map<const char*, ComponentType>::iterator getComponentTypeIteratorEnd() {
			return componentTypes.end();
		}

		std::shared_ptr<ComponentArray<SerializableComponent>> GetComponentArraySerializable(const char* typeName) {
			return std::static_pointer_cast<ComponentArray<SerializableComponent>>(componentArrays[typeName]);
		}

		//changing key from const char* to std::string will make this function O(1) 
		//In fact you wouldn't even need this function
		std::shared_ptr<ComponentArray<SerializableComponent>> GetComponentArraySerializableByComp(const char* typeName) {
			for (auto it = componentArrays.begin(); it != componentArrays.end(); it++) {
				if (strcmp(it->first, typeName) == 0) {
					return std::static_pointer_cast<ComponentArray<SerializableComponent>>(componentArrays[it->first]);
				}
			}
		}

	private:
		// Map from type string pointer to a component type
		std::unordered_map<const char*, ComponentType> componentTypes{};

		// Map from type string pointer to a component array
		std::unordered_map<const char*, std::shared_ptr<IComponentArray>> componentArrays{};

		// The component type to be assigned to the next registered component - starting at 0
		ComponentType nextComponentType{};

		// Convenience function to get the statically casted pointer to the ComponentArray of type T.
		template<typename T>
		std::shared_ptr<ComponentArray<T>> GetComponentArray()
		{
			const char* typeName = typeid(T).name();
			assert(componentTypes.find(typeName) != componentTypes.end() && "Component not registered before use.");
			return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
		}

	};

	class Registry {
	public:
		Registry(){
			// Initialize the queue with all possible entity IDs
			for (Entity entity = 0; entity < MAX_ENTITIES; ++entity)
			{
				entityIDQueue.push(entity);
			}
		}

		Entity CreateEntity()
		{
			assert(entityCount < MAX_ENTITIES && "Too many entities in existence.");
			// Take an ID from the front of the queue
			Entity id = entityIDQueue.front();
			entityIDQueue.pop();
			++entityCount;

			return id;
		}

		void DestroyEntity(Entity entity)
		{
			assert(entity < MAX_ENTITIES && "Entity out of range.");
			signatures[entity].reset();
			// Put the destroyed ID at the back of the queue
			entityIDQueue.push(entity);
			--entityCount;
		}

		void SetSignature(Entity entity, Signature signature)
		{
			assert(entity < MAX_ENTITIES && "Entity out of range.");
			// Put this entity's signature into the array
			signatures[entity] = signature;
		}

		Signature GetSignature(Entity entity)
		{
			assert(entity < MAX_ENTITIES && "Entity out of range.");
			// Get this entity's signature from the array
			return signatures[entity];
		}

	private:
		uint32_t entityCount = 0;
		
		// Array of signatures where the index corresponds to the entity ID
		// Signature, bitset indicates which component an entity has
		std::array<Signature, MAX_ENTITIES> signatures{};
		std::queue<Entity> entityIDQueue;
	};

	// Keeps track of changes of entities so that each entity systems can have a small set of entities that needs to loop over
	class SystemManager
	{
	public:
		template<typename T, class... types>
		std::shared_ptr<T> RegisterSystem(types&&... _Args)
		{
			const char* typeName = typeid(T).name();

			assert(mSystems.find(typeName) == mSystems.end() && "Registering system more than once.");

			// Create a pointer to the system and return it so it can be used externally
			auto system = std::make_shared<T>(_Args...);
			mSystems.insert({ typeName, system });
			return system;
		}

		template<typename T>
		void SetSignature(Signature signature)
		{
			const char* typeName = typeid(T).name();

			assert(mSystems.find(typeName) != mSystems.end() && "System used before registered.");

			// Set the signature for this system
			mSignatures.insert({ typeName, signature });
		}

		void EntityDestroyed(Entity entity)
		{
			// Erase a destroyed entity from all system lists
			// mEntities is a set so no check needed
			for (auto const& pair : mSystems)
			{
				auto const& system = pair.second;

				system->mEntities.erase(entity);
			}
		}

		void EntitySignatureChanged(Entity entity, Signature const& entitySignature)
		{
			// Notify each system that an entity's signature changed
			for (auto const& pair : mSystems)
			{
				auto const& type = pair.first;
				auto const& system = pair.second;
				Signature const& systemSignature = mSignatures[type];

				// Entity signature matches system signature - insert into set
				if ((entitySignature & systemSignature) == systemSignature)
				{
					system->mEntities.insert(entity);
				}
				// Entity signature does not match system signature - erase from set
				else
				{
					system->mEntities.erase(entity);
				}
			}
		}

	private:
		// Map from system type string pointer to a signature
		std::unordered_map<const char*, Signature> mSignatures{};

		// Map from system type string pointer to a system pointer
		std::unordered_map<const char*, std::shared_ptr<EntitySystem>> mSystems{};
	};

	class Coordinator
	{
	public:
		static Coordinator* GetCoordinator();

		static void DeleteCoordinator();

		void Init()
		{
			// Create pointers to each manager
			componentManager = std::make_unique<ComponentManager>();
			registry = std::make_unique<Registry>();
			systemManager = std::make_unique<SystemManager>();
		}

		// Entity methods
		Entity CreateEntity() { return registry->CreateEntity(); }

		void DestroyEntity(Entity entity)
		{
			registry->DestroyEntity(entity);
			componentManager->EntityDestroyed(entity);
			systemManager->EntityDestroyed(entity);
		}

		// Component methods
		template<typename T>
		void RegisterComponent() { componentManager->RegisterComponent<T>(); }

		template<typename T>
		T* AddComponent(Entity entity, T component)
		{
			T* comp = componentManager->AddComponent<T>(entity, component);

			auto signature = registry->GetSignature(entity);
			signature.set(componentManager->GetComponentType<T>(), true);
			registry->SetSignature(entity, signature);

			systemManager->EntitySignatureChanged(entity, signature);
			return comp;
		}

		template<typename T>
		void RemoveComponent(Entity entity)
		{
			componentManager->RemoveComponent<T>(entity);

			auto signature = registry->GetSignature(entity);
			signature.set(componentManager->GetComponentType<T>(), false);
			registry->SetSignature(entity, signature);

			systemManager->EntitySignatureChanged(entity, signature);
		}

		template<typename T>
		T& GetComponent(Entity entity) { return componentManager->GetComponent<T>(entity); }

		template<typename T>
		ComponentType GetComponentType() { return componentManager->GetComponentType<T>(); }
		
		// System methods
		template<typename T, class... T_initializers>
		std::shared_ptr<T> RegisterSystem(T_initializers&&... args) { return systemManager->RegisterSystem<T>(this, args...); }

		template<typename T>
		void SetSystemSignature(Signature signature) { systemManager->SetSignature<T>(signature); }

		void Serialize();
		void Deserialize();

		Coordinator() = default;
		Coordinator(const Coordinator&) = delete;
		Coordinator operator=(const Coordinator&) = delete;

	private:
		static Coordinator* _globalCoordinator;

		std::unique_ptr<ComponentManager> componentManager;
		std::unique_ptr<Registry> registry;
		std::unique_ptr<SystemManager> systemManager;
	};
}