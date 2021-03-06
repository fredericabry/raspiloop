#ifndef CAPTURE_PORT_C_H
#define CAPTURE_PORT_C_H



#include <QMainWindow>
#include <qmutex.h>



class interface_c;





class capture_port_c : public QObject
{
    Q_OBJECT

public:
    capture_port_c(const unsigned long maxlength,unsigned long bufsize, const int channel, const QString deviceName, interface_c *interface);
    ~capture_port_c();

    const unsigned long maxlength;
    const int channel;
    const QString deviceName;
    interface_c *interface;

    short *ringbuf;//large buffer for circular storage
    short *bufin;//small buffer for transfert to the ringbuffer

    unsigned long head;
    QMutex ring_lock;
    void pushN(unsigned long N);


};


#endif // CAPTURE_PORT_C_H
