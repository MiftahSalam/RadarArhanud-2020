#ifndef RADARSCENE_H
#define RADARSCENE_H

#include <QGraphicsScene>
#include "arpatrackitem.h"
#include "ifftrackitem.h"

class RadarScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit RadarScene(QObject *parent = 0, RI *ri_ptr=0);

signals:

protected:

private slots:
    void trigger_reqNewArpa(bool create, int last_count);

private:
    RI *m_ri;


};

#endif // RADARSCENE_H
