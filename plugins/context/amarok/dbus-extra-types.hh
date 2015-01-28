
struct DBusStatus
{
    int Play; //Playing = 0, Paused = 1, Stopped = 2
    int Random; //Linearly = 0, Randomly = 1
    int Repeat; //Go_To_Next = 0, Repeat_Current = 1
    int RepeatPlaylist; //Stop_When_Finished = 0, Never_Give_Up_Playing = 1
};

Q_DECLARE_METATYPE( DBusStatus )

// Marshall the DBusStatus data into a D-BUS argument
QDBusArgument &operator << ( QDBusArgument &argument, const DBusStatus &status );
// Retrieve the DBusStatus data from the D-BUS argument
const QDBusArgument &operator >> ( const QDBusArgument &argument, DBusStatus &status );
