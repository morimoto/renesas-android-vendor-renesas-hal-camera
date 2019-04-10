#ifndef ANDROID_HARDWARE_CAMERA_PROVIDER_V2_4_UEVENTLISTENER_H
#define ANDROID_HARDWARE_CAMERA_PROVIDER_V2_4_UEVENTLISTENER_H

#include <map>
#include <string>
#include <functional>



class Listener
{
    public:
        Listener    () = default;
        ~Listener   () = default;

        int Loop    ();

        using on_change_cb = std::function <void (const char *)>;

        void SetAddCallback    (on_change_cb cb) {onAdd    = cb;}
        void SetRemoveCallback (on_change_cb cb) {onRemove = cb;}

    private:
        // Key - path on sysfs, value - path on devfs.
        std::map <std::string, std::string> devices;

        on_change_cb onAdd;
        on_change_cb onRemove;
};

#endif//ANDROID_HARDWARE_CAMERA_PROVIDER_V2_4_UEVENTLISTENER_H
