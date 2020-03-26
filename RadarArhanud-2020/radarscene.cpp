#include "radarscene.h"

#include <QDebug>

RadarScene::RadarScene(QObject *parent,RI *ri_ptr) :
    QGraphicsScene(parent),m_ri(ri_ptr)
{
    qDebug()<<Q_FUNC_INFO<<m_ri->m_arpa;
}


void RadarScene::trigger_reqNewArpa(bool create, int id)
{
    qDebug()<<Q_FUNC_INFO<<create<<id<<m_ri->m_arpa->m_target[id];

    if(create)
    {
        addItem(new ArpaTrackItem(m_ri->m_arpa->m_target[id]));
    }
}
