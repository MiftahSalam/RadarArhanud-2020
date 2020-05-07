#include "radarscene.h"

#include <QDebug>

RadarScene::RadarScene(QObject *parent,RA *ra_ptr) :
    QGraphicsScene(parent),m_ra(ra_ptr)
{
    qDebug()<<Q_FUNC_INFO<<m_ra;
}


void RadarScene::trigger_reqNewArpa(bool create, int last_count)
{
    qDebug()<<Q_FUNC_INFO<<create<<last_count<<m_ra->m_target[last_count];
    qDebug()<<Q_FUNC_INFO<<items().size();

    if(create)
    {
        addItem(new ArpaTrackItem(m_ra->m_target[last_count]));
    }
}
