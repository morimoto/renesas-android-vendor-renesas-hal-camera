#define LOG_TAG "CamPvdrListener@2.4-external.kingfisher"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cerrno>
#include <string.h>
#include <poll.h>
#include <cutils/uevent.h>
#include <log/log.h>
#include "ExternalCameraUeventListener.h"



// Probing stage.
enum class State
{
    Unknown
  , Added
  , Binded
  , Removed
};



// Helper. Find a required key (tag) and return a pointer on a value.
const char * FindTag (const char * p, const std::string & tag)
{
    while (* p) {
        if (!std::strncmp (p, tag.c_str (), tag.length () ) )
            return p + tag.length ();
        while (* p++)
            ;
    }
    return nullptr;
}



int Listener::Loop ()
{
    constexpr int ueventMsgLen {4096};  // Maximul length of input message.
    char message [ueventMsgLen + 2];    // Input buffer for messages.

    const std::string actionTag {"ACTION="};
    const std::string systemTag {"SUBSYSTEM="};
    const std::string pathTag   {"DEVPATH="};


    // Open netlink socket.
    int nd = -1;    // Netlink socket descriptor.
    nd = uevent_open_socket (1 * 1024 * 1024, true);
    if (-1 == nd) {
        ALOGE ("Can not open a socket: %s.", std::strerror (errno) );
        return errno;
    }
    pollfd fds;
    fds.events = POLLIN;
    fds.fd     = nd;


    // Event loop.
    while (true/*forever*/) {
        // Blocking wait for a message.
        fds.revents = 0;
        if (poll (& fds, 1, -1) < 0) {
            ALOGE ("Poll failed: %s.", std::strerror (errno) );

            return errno;
        }


        // Check the message type.
        if (!(fds.revents & POLLIN) ) {
            continue;
        }


        // Get the message.
        const int n = uevent_kernel_multicast_recv (nd, message, ueventMsgLen);
        if (n <= 0) {
            ALOGE ("Read failed: %s.", std::strerror (errno) );
        }
        if (n >= ueventMsgLen) {
            ALOGE ("Buffer overflow.");
        }
        message [n + 0] = '\0';
        message [n + 1] = '\0';


        // Parse the message, get "action".
        State state = State::Unknown;
        if (const char * p = FindTag (message, actionTag) ) {
            if (!std::strncmp (p, "add",    3) ) state = State::Added;
            if (!std::strncmp (p, "bind",   4) ) state = State::Binded;
            if (!std::strncmp (p, "remove", 6) ) state = State::Removed;
        }
        if (State::Unknown == state) {
            continue;
        }


        // Parse the message, get "path".
        std::string path;
        if (const char * p = FindTag (message, pathTag) ) {
            path = std::string (p);
        }


        switch (state) {
            case State::Removed: {
                if (devices.erase (path) ) {
                    ALOGI ("Controlled device is unplugged.");
                    if (onRemove) {
                        onRemove (path.c_str () );
                    }
                }
                break;
            }

            case State::Added: {
                if (const char * p = FindTag (message, systemTag) ) {
                    if (!std::strncmp (p, "video4linux", 11) ) {
                        if (devices.count (path) == 0) {
                            devices.insert ({path, ""});
                        }
                    }
                }
                break;
            }

            case State::Binded: {
                auto x = std::find_if (devices.begin (), devices.end (),
                    [& path](const std::pair <std::string, std::string> & d) {
                        return !d.first.compare (0, path.length (), path);
                    }
                );

                if (devices.end () == x) {
                    ALOGI ("Binded device isn't under control.");
                    break;
                }

                if (!x->second.empty () ) {
                    ALOGI ("Binded device has been already reported.");
                    break;
                }

                const std::string devPath = std::string ("/dev") + x->first.substr (x->first.rfind ('/') );
                devices.insert ({path, devPath});
                ALOGI ("Device is plugged: %s, reported path: %s.", x->first.c_str (), devPath.c_str () );
                if (onAdd) {
                    onAdd (devPath.c_str () );
                }
            }

            default:
                ;
        }
    }

    // We should not go out here.
    return -EINVAL;
}
