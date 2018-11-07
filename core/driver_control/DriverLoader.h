#include "../base/types.h"

#include "../registry/UserRegistry.hpp"

/*
use load() to load driver.
driver will be auto unloaded when object is destructed.
*/
class DriverLoader
{
public:
    DriverLoader(const estr_t &service_name, const estr_t &driver_path)
        :service_name(service_name), driver_path(driver_path)
    {
    }
    ~DriverLoader()
    {
        stop();
        unload();
    }
public:
    bool create()
    {
        if (!created)
            created = create_driver_service(service_name, driver_path);
        return created;
    }
    bool load()
    {
        create();
        if (!loaded)
        {
            loaded = start_service(this->service_name);
        }
        return loaded;
    }
    bool stop()
    {
        if (loaded)
        {
            loaded = !stop_service(service_name);
        }
        return !loaded;
    }
    bool unload()
    {
        stop();
        if (created)
        {
            created = !delete_service(this->service_name);
        }
        return !loaded && !created;
    }
    inline UserRegistry registry()
    {
        estr_t path = make_estr(LR"(\Registry\Machine\SYSTEM\CurrentControlSet\Services\)");
        path += service_name;
        return UserRegistry(path);
    }
public:
    static bool create_driver_service(const estr_t &eservice_name, const estr_t &driver_path);
    static bool start_service(const estr_t &eservice_name);
    static bool stop_service(const estr_t &eservice_name);
    static bool delete_service(const estr_t &eservice_name);
private:
    bool created = false;
    bool loaded = false;
    estr_t driver_path;
    estr_t service_name;
};