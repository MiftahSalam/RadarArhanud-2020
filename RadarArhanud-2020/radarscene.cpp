#include "radarscene.h"
#include "arpatrackitem.h"
#include "adsbtrackitem.h"

#include <QDebug>
#include <qmath.h>

const char fShader [] =
        "uniform sampler2D texture1;\n"
        "varying mediump vec4 texc;\n"
        "void main(void)\n"
        "{\n"
        "gl_FragColor = texture2D(texture1, texc.st);\n"
        "};\n"
        ;

const char vShader [] =
        "attribute highp vec4 aPos;\n"
        "attribute mediump vec4 aTexCoord;\n"
        "varying mediump vec4 texc;\n"
        "void main(void)\n"
        "{\n"
            "gl_Position = aPos;\n"
            "texc = aTexCoord;\n"
        "}\n"
        ;

RadarScene::RadarScene(QObject *parent, RadarEngineARND::RadarEngine *ri_ptr) :
    QGraphicsScene(parent),m_ri(ri_ptr),curScale(1.0f),curAngle(0.)
{
    initGL();

    currentCursor.cursorMoveTime = QTime::currentTime().addSecs(-5);

    m_timer = new QTimer(this);
    m_timer->setInterval(100);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_timer->start();

}

void RadarScene::DrawSpoke(int angle, u_int8_t *data, size_t len)
{
    if(angle == 2046)
        emit signal_zero_detect();

    curAngle = SCALE_RAW_TO_DEGREES2048(angle);
    m_ri->radarDraw->ProcessRadarSpoke(angle,data,len);
    update();
}

void RadarScene::setRadarScale(float scale)
{
    qDebug()<<Q_FUNC_INFO<<scale;
    curScale = scale;
}

void RadarScene::trigger_mapChange(QImage img)
{
    mapImage = img;
    m_text->load(mapImage);
    qDebug()<<Q_FUNC_INFO<<img.size();
}

void RadarScene::drawBackground(QPainter *painter, const QRectF &)
{
//    qDebug()<<Q_FUNC_INFO;

    int width = painter->device()->width();
    int height = painter->device()->height();
    int side = qMax(width,height);
    int side_min = qMin(width,height)/2;

    glViewport(0,0,width,height);

    painter->beginNativePainting();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    saveGLState();

    if(map_settings.show)
    {
        vbo.create();
        vbo.bind();
        vbo.allocate(vertData.constData(), vertData.count() * sizeof(GLfloat));

        m_environmentProgram->enableAttributeArray(0);
        m_environmentProgram->enableAttributeArray(1);
        m_environmentProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3, 5 * sizeof(GLfloat));
        m_environmentProgram->setAttributeBuffer(1, GL_FLOAT, 3 * sizeof(GLfloat), 2, 5 * sizeof(GLfloat));

        m_environmentProgram->bind();
        m_text->bind();
        glDrawArrays(GL_TRIANGLE_FAN,0,4);
        m_text->unbind();
        vbo.release();
        m_environmentProgram->disableAttributeArray(0);
        m_environmentProgram->disableAttributeArray(1);
        m_environmentProgram->release();
    }


    if(state_radar == RADAR_TRANSMIT)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glViewport((width - side) / 2,(height - side) / 2,side,side);
        glLoadIdentity();
//        curScale = 1.0f; //temporary
        glScaled(curScale, curScale, 1.);

        m_ri->radarDraw->DrawRadarImage();
        glDisable(GL_BLEND);

        glBegin(GL_LINES);
        glColor3f(0,1,0);
        glVertex2f(0,0);
        glVertex2f(sin(static_cast<float>(deg2rad(curAngle))),
                   cos(static_cast<float>(deg2rad(curAngle))));
        glEnd();
    }

    QFont font;
    QPen pen;

    painter->translate(width/2,height/2);

    //compass ring text
    pen.setWidth(3);

    pen.setColor(Qt::yellow);

    if(map_settings.show && map_settings.mode == 0)
        pen.setColor(Qt::yellow);
    else if(map_settings.show && map_settings.mode == 1)
        pen.setColor(Qt::black);

    painter->setPen(pen);

    if(radar_settings.show_compass)
    {
        QString text;
        for(int j=0;j<12;j++)
        {
            if(j<9)
                QTextStream(&text)<<(j*30)+90;
            else
                QTextStream(&text)<<(j*30)-270;

            int marginY = 5;
            int marginX = 15;
            QRect rect1((side_min-20)*cos((j*30)*M_PI/180)-marginX,
                        ((side_min-20)*sin((j*30)*M_PI/180)-marginY),
                        30,
                        15);
            QTextOption opt;
            opt.setAlignment(Qt::AlignHCenter);
            QFont font;

            font.setPixelSize(15);
            painter->setFont(font);
            painter->drawText(rect1,text,opt);
            text.clear();
        }

        //compass ring
        for(int j=0;j<180;j++)
        {
            int marginBig = 10;
            int marginSmall = 5;

            if(j%15==0)
                painter->drawLine(0,side_min,0,side_min-marginBig);
            else
                painter->drawLine(0,side_min,0,side_min-marginSmall);

            painter->rotate(2);
        }
    }

    //Range rings
    if(radar_settings.show_rings)
    {
        pen = painter->pen();
        pen.setWidth(1);

        painter->setPen(pen);

        int ring_margin = qCeil(side/5);
        int bufRng = ring_margin;
        while(bufRng < side)
        {
            painter->drawEllipse(-bufRng/2,-bufRng/2,bufRng,bufRng);
            bufRng += ring_margin;
        }
    }

    //Cursor position
    QTime now = QTime::currentTime();
    if(currentCursor.cursorMoveTime.secsTo(now) < 5)
    {
        //        qDebug()<<Q_FUNC_INFO<<cursorMoveTime.secsTo(now);

        pen = painter->pen();
        pen.setWidth(3);

        if(map_settings.show)
            pen.setColor(Qt::black);

        font.setPixelSize(15);
        font.setBold(true);

        painter->setPen(pen);
        painter->setFont(font);
        painter->drawText((-width/2)+10,(height/2)-50,"Cursor:");
        QString lat_lon = QString("Latitude : %1 \t Longitude: %2")
                .arg(QString::number(currentCursor.latitude,'f',6))
                .arg(QString::number(currentCursor.longitude,'f',6));
        QString rng_brn = QString("Range : %1 Km \t Bearing: %2%3")
                .arg(QString::number(currentCursor.range,'f',2))
                .arg(QString::number(currentCursor.bearing,'f',2))
                .arg(176);
        painter->drawText((-width/2)+10,(height/2)-35,lat_lon);
        painter->drawText((-width/2)+10,(height/2)-20,rng_brn);
    }

    //loading map status
    if(map_settings.show)
    {
        if(map_settings.loading)
        {
            if(map_settings.mode == 0)
                pen.setColor(Qt::yellow);
            else if(map_settings.mode == 1)
                pen.setColor(Qt::black);

            font.setPixelSize(20);
            font.setBold(true);

            painter->setPen(pen);
            painter->setFont(font);

            painter->drawText(-50,-20,"Loading Map");
        }
    }

    //heading marker
    if(radar_settings.show_heading_marker)
    {
        pen = painter->pen();
        pen.setWidth(3);

        if(map_settings.show && map_settings.mode == 0)
            pen.setColor(Qt::yellow);
        else if(map_settings.show && map_settings.mode == 1)
            pen.setColor(Qt::black);

        painter->setPen(pen);
        painter->rotate(currentHeading);
        painter->drawLine(0,0,0,-side);
        painter->rotate(-currentHeading);
    }

    /*
      Radar status
*/
    if(state_radar != RADAR_TRANSMIT)
    {
        QString text;
        QTextOption opt;
        opt.setAlignment(Qt::AlignHCenter);
        QFont font;

        font.setPixelSize(32);
        painter->setFont(font);

        switch (state_radar)
        {
        case RADAR_OFF:
            text = "No Radar";
            break;
        case RADAR_WAKING_UP:
            text = "Waking Up";
            break;
        case RADAR_STANDBY:
            text = "Standby";
            break;
        case RADAR_NO_SPOKE:
            text = "Warming Up";
            break;
        default:
            break;
        }


        QFontMetrics metric = QFontMetrics(font);
        QRect rect = metric.boundingRect(0,0,side, int(side*0.125),
                                          Qt::AlignCenter | Qt::TextWordWrap, text);

        painter->drawText(-rect.width()/2,5,rect.width(), rect.height(),Qt::AlignCenter | Qt::TextWordWrap, text);
    }

    restoreGLState();
    painter->endNativePainting();
}

void RadarScene::initGL()
{
    m_vertexShader = new QGLShader(QGLShader::Vertex);
    m_vertexShader->compileSourceCode(vShader);

    m_fragmentShaders = new QGLShader(QGLShader::Fragment);
    m_fragmentShaders->compileSourceCode(fShader);

    m_environmentProgram = new QGLShaderProgram;
    m_environmentProgram->addShader(m_vertexShader);
    m_environmentProgram->addShader(m_fragmentShaders);
    m_environmentProgram->bindAttributeLocation("aPos", 0);
    m_environmentProgram->bindAttributeLocation("aTexCoord", 1);
    m_environmentProgram->link();
    m_environmentProgram->bind();
    m_environmentProgram->setUniformValue("texture1", 0);

    /*
    float vertices[4][8] = {
        {-1.f,  1.f, 0.0f,   0.25f, 0.25f},
        {-1.f, -1.f, 0.0f,   0.25f, .75f},
        {1.f, -1.f, 0.0f,  0.75f, 0.75f},
        {1.f,  1.f, 0.0f,  .75f, 0.25f}
    };
    */
    /*
    */
    float vertices[4][8] = {
        {-1.f,  1.f, 0.0f,   0.f, 0.f},
        {-1.f, -1.f, 0.0f,   0.f, 1.0f},
        {1.f, -1.f, 0.0f,  1.0f, 1.0f},
        {1.f,  1.f, 0.0f,  1.0f, 0.f}
    };
    for(int j = 0; j < 4; ++j)
    {
        vertData.append(vertices[j][0]);
        vertData.append(vertices[j][1]);
        vertData.append(vertices[j][2]);

        vertData.append(vertices[j][3]);
        vertData.append(vertices[j][4]);
    }

    m_text = new GLTextureCube(sceneRect().width(),sceneRect().height());

    glClearColor(0.f, 0.0f, 0.0f, 1.0f);
#ifdef QT_OPENGL_ES
    glOrthof(-0.5, +0.5, -0.5, 0.5, 4.0, 15.0);
#else
    glOrtho(-0.5, +0.5, -0.5, 0.5, 4.0, 15.0);
#endif

}

void RadarScene::saveGLState()
{
    glEnable(GL_NORMALIZE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
}

void RadarScene::restoreGLState()
{
    glDisable(GL_NORMALIZE);

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

RadarScene::~RadarScene()
{

}

void RadarScene::trigger_cursorPosition(qreal lat, qreal lon, qreal rng, qreal brn)
{
//    qDebug()<<Q_FUNC_INFO;

    currentCursor.cursorMoveTime = QTime::currentTime();
    currentCursor.latitude = lat;
    currentCursor.longitude = lon;
    currentCursor.range = rng;
    currentCursor.bearing = brn;
}

void RadarScene::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug()<<Q_FUNC_INFO;
}

void RadarScene::reqNewArpa(bool create, bool show, ARPATarget *arpa_ptr)
{
    qDebug()<<Q_FUNC_INFO<<create<<arpa_ptr;
    qDebug()<<Q_FUNC_INFO<<items().size();

    if(create)
    {
        ArpaTrackItem *newArpaTrack = new ArpaTrackItem(arpa_ptr);
        newArpaTrack->setShow(show);
        addItem(newArpaTrack);
    }
    /*
    */
}

void RadarScene::reqNewADSB(AdsbArhnd::ADSBTargetData* target, bool show)
{
//    qDebug()<<Q_FUNC_INFO<<target;
    AdsbTrackItem *newAdsbTrack = new AdsbTrackItem(target);
    newAdsbTrack->setShow(show);
    addItem(newAdsbTrack);
}
