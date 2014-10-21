#ifndef DATAHOLDER_H
#define DATAHOLDER_H

#include "Representation.h"
#include "Serializer.h"

/**
 * Connects a arbitrary class with Representation
 */
template<class T>
class DataHolder: public Representation {
private:
	// creates an object of the data
	// (this requires the class T to have a default constructor)
	T data;

public:
	DataHolder(const std::string& name)
		: Representation(name)
	{}

	DataHolder()
		: Representation(typeid(T).name())
	{}

	virtual ~DataHolder() {}

	T& operator*() {
		return data;
	}

	const T& operator*() const {
		return data;
	}

	virtual bool serialize(SERIALIZER &archive) override {
		return Serializer<T>::serialize(data, archive);
	}

	virtual bool deserialize(DESERIALIZER &archive) override {
		return Serializer<T>::deserialize(data, archive);
	}
};

#endif /* DATAHOLDER_H */
