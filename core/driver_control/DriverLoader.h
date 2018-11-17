#include "../base/types.h"

#include "../registry/UserRegistry.hpp"
#include "../base/fs/fsutils.h"

/*
use load() to load driver.
driver will be auto unloaded when object is destructed.
*/
class DriverLoader {
public:
    DriverLoader(const estr_t &service_name, const estr_t &driver_path)
        :_service_name(service_name), _driver_path(driver_path)
    {}
    ~DriverLoader()
    {
        stop();
        unload();
    }
public:
    bool create()
    {
        if (!created)
        {
            created = create_driver_service(_service_name, _driver_path);
        }
        return created;
    }
    bool load()
    {
        if (create())
        {
            if (!loaded)
            {
                loaded = start_service(this->_service_name);
            }
        }
        return loaded;
    }
    bool stop()
    {
        if (loaded)
        {
            loaded = !stop_service(_service_name);
        }
        return !loaded;
    }
    bool unload()
    {
        if (stop())
        {
            if (created)
            {
                created = !delete_service(this->_service_name);
            }
        }
        return !loaded && !created;
    }
    const estr_t &driver_path() const noexcept
    {
        return _driver_path;
    }
    estr_t driver_filename()
    {
        return pkn::filename_for_path(_driver_path);
    }
    inline UserRegistry registry()
    {
        estr_t path = make_estr(R"(\Registry\Machine\SYSTEM\CurrentControlSet\Services\)");
        path += _service_name;
        return UserRegistry(path);
    }
public:
    static bool create_driver_service(const estr_t &eservice_name, const estr_t &driver_path);
    static bool start_service(const estr_t &eservice_name);
    static bool stop_service(const estr_t &eservice_name);
    static bool delete_service(const estr_t &eservice_name);
    static bool enable_privilege(const char *name);
    static bool enable_load_driver_privilege();
public:
    bool created = false;
    bool loaded = false;
    estr_t _driver_path;
    estr_t _service_name;
};