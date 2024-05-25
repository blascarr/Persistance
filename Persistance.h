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

#if !defined(NVS) && defined(SPIFFS_STORAGE)
#include <FS.h>
#define FILESYSTEM SPIFFS
#define FILESYSTEMNAME "SPIFFS"
#endif

#if !defined(NVS) && !defined(SPIFFS_STORAGE)
#include <LittleFS.h>
#define FILESYSTEM LittleFS
#define FILESYSTEMNAME "LittleFS"
#endif

#if !defined(NVS)
class FS_Storage : public IStorage {
  public:
	FS_Storage() {
		if (!FILESYSTEM.begin()) {
			Serial.println(FILESYSTEMNAME);
			Serial.println("LittleFS mount failed");
		}
	}
	~FS_Storage() { FILESYSTEM.end(); }
	void save(const String &data, const String &path) override {
		File file = FILESYSTEM.open(path, "w");
		if (!file) {
			Serial.println("Failed to open file for writing");
			return;
		}
		file.println(data);
		file.close();
	}

	String load(const String &path) override {
		File file = FILESYSTEM.open(path, "r");
		if (!file) {
			Serial.println("Failed to open file for reading");
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

/*
#if defined(ESP32) && !defined(NVS)
class FS_Storage : public IStorage {
  public:
	FS_Storage() {
		if (!FILESYSTEM.begin()) {
			Serial.println("LittleFS mount failed");
		}
	}
	~FS_Storage() { FILESYSTEM.end(); }
	void save(const String &data, const String &path) override {
		File file = FILESYSTEM.open(path, "w");
		if (!file) {
			Serial.println("Failed to open file for writing");
			return;
		}
		file.println(data);
		file.close();
	}

	String load(const String &path) override {
		File file = FILESYSTEM.open(path, "r");
		if (!file) {
			Serial.println("Failed to open file for writing");
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
*/

#if defined(NVS)
#include <Preferences.h>

class NVS_Storage : public IStorage {
  public:
	void save(const String &data, const String &path) override {
		preferences.begin("my-app", false);
		preferences.putString(path, data);
		preferences.end();
	}

	String load(const String &path) override {
		String data = preferences.getString(path);
		preferences.end();
		return data;
	}

	void remove(const String &path) {
		preferences.remove(path);
		preferences.end();
	}
	void removeAll() {
		preferences.clear();
		preferences.end();
	}
};
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