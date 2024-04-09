#ifndef _PERSISTANCE_H
#define _PERSISTANCE_H

#include <Arduino.h>
#include <ArduinoJson.h>

class IStorage {
  public:
	virtual ~IStorage() {}
	virtual void save(const String &data, const String &path) = 0;
	virtual String load(const String &path) = 0;
	virtual void saveJSON(const JsonDocument &data, const String &path) = 0;
	virtual JsonDocument loadJSON(const String &path) = 0;
};

class IStringSerializable {
  public:
	virtual String serialize() { return ""; };
	virtual bool deserialize(const String &data) { return false; };
};

class IJSONSerializable {
  public:
	virtual JsonDocument serializeJSON() {
		JsonDocument doc;
		return doc;
	};
	virtual bool deserializeJSON(const String &data) { return false; };
};

class ISerializable : public IStringSerializable, public IJSONSerializable {};

#if defined(ESP8266)
#include <LittleFS.h>
class LittleFS_Storage : public IStorage {
  public:
	LittleFS_Storage() {
		if (!LittleFS.begin()) {
			// DUMPSLN("LittleFS mount failed");
		}
	}
	~LittleFS_Storage() { LittleFS.end(); }
	void save(const String &data, const String &path) override {
		File file = LittleFS.open(path, "w");
		if (!file) {
			// DUMPSLN("Failed to open file for writing");
			return;
		}
		file.println(data);
		file.close();
	}

	String load(const String &path) override {
		File file = LittleFS.open(path, "r");
		if (!file) {
			// DUMPSLN("Failed to open file for reading");
			return "";
		}
		String data = file.readStringUntil('\n');
		file.close();
		return data;
	}
	void saveJSON(const JsonDocument &data, const String &path) override {}
	JsonDocument loadJSON(const String &path) override {
		JsonDocument doc;
		return doc;
	}
};
#endif

#if defined(ESP32)
#include <FS.h>
class FS_Storage : public IStorage {
  public:
	void save(const String &data, const String &path) override {}
	String load(const String &path) override { return ""; }
};
#endif

#if defined(SPIFFS_STORAGE)
class SPIFFS_Storage : public IStorage {
  public:
	void save(const String &data, const String &path) override {}

	String load(const String &path) override { return ""; }
};
#endif

#if defined(PREFERENCE_STORAGE)

#endif

class Persistance {
  private:
	IStorage *storageModel = nullptr;
	ISerializable *datasource = nullptr;

  public:
	Persistance(ISerializable *data) : datasource(data) {}
	Persistance(ISerializable *data, IStorage *storage)
		: datasource(data), storageModel(storage) {}

	void setStorageModel(IStorage *model) { storageModel = model; }
	void setDataSource(ISerializable *data) { datasource = data; }
	void saveData(const String &path) {
		if (storageModel && datasource) {
			String data = datasource->serialize();
			storageModel->save(data, path);
		}
		// TODO: ADD Exception no storage or datasource
	}

	void saveDataJSON(const String &path) {
		if (storageModel && datasource) {
			String data = datasource->serialize();
			storageModel->save(data, path);
		}
		// TODO: ADD Exception no storage or datasource
	}

	String loadData(const String &path) {
		if (storageModel && datasource) {
			String data = storageModel->load(path);
			if (!data.isEmpty()) {
				datasource->deserialize(data);
				return data;
			}
			// TODO: ADD Exception data Empty
		}
		// TODO: ADD Exception no storage or datasource
	}
};

#endif