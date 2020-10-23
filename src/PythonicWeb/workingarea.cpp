/*
 * This file is part of Pythonic.

 * Pythonic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * Pythonic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with Pythonic. If not, see <https://www.gnu.org/licenses/>
 */

#include "workingarea.h"



WorkingArea::WorkingArea(int gridNo, QWidget *parent)
    : QFrame(parent)
    , m_gridNo(gridNo)
    , logC("WorkingArea")
{
    setAcceptDrops(true);
    setMouseTracking(true);
    //setObjectName("workBackground");
    QString objectName = QString("workingArea_%1").arg(gridNo);

    setObjectName(objectName);



    /* Setup background */

    m_backgroundGradient.setStart(0.0, 0.0);
    m_backgroundGradient.setColorAt(0,      BACKGROUND_COLOR_A);
    m_backgroundGradient.setColorAt(0.5,    BACKGROUND_COLOR_B);
    m_backgroundGradient.setColorAt(1.0,    BACKGROUND_COLOR_C);

    /* Setup connections settings */

    m_pen.setColor(CONNECTION_COLOR);
    m_pen.setWidth(CONNECTION_THICKNESS);



    qCDebug(logC, "called");
}

void WorkingArea::updateSize()
{
    qCInfo(logC, "called");
}

void WorkingArea::deleteElement(ElementMaster *element)
{
    qCInfo(logC, "called");
    delete element;
#if 0
    QObjectList objectList = children();

    if(objectList.contains(element)){
        qCDebug(logC, "Element gefunden");
        delete element;
    } else {
        qCDebug(logC, "Element nicht gefunden");
    }



#endif


}

void WorkingArea::registerElement(const ElementMaster *new_element)
{
    qCDebug(logC, "called with element %s", new_element->objectName().toStdString().c_str());
    connect(new_element, &ElementMaster::remove,
            this, &WorkingArea::deleteElement);
}



void WorkingArea::mousePressEvent(QMouseEvent *event)
{
    qCDebug(logC, "Event Position: X: %d Y: %d", event->x(), event->y());
    QLabel *child = qobject_cast<QLabel*>(childAt(event->pos()));


    if (!child){
        //qCDebug(logC, "called - no child");
        return;
    }

    /*
     *  Hierarchy of ElementMaster
     *
     *  m_symbol(QLabel) --(parent)-->
     *                   m_symbolWidget(QWidget) --(parent)-->
     *                                           m_innerWidget(QLabel) --(parent)-->
     *                                                                 ElementMaster
     */
    m_tmpElement= qobject_cast<ElementMaster*>(child->parent()->parent()->parent());

    if (!m_tmpElement){
        qCDebug(logC, "master not found");
        return;
    } else if (m_tmpElement->m_plug.underMouse()){
        // begin drawing
        m_draw = true;
        m_previewConnection = QLine(); // reset connection
        m_drawStartPos = event->pos(); // set start position
        qCDebug(logC, "Plug under Mouse: %d", m_tmpElement->m_plug.underMouse());
    } else if (m_tmpElement->m_socket.underMouse()){
        return;
    } else {
        this->setCursor(Qt::OpenHandCursor);
        m_dragging = true;

        qCDebug(logC, "Widget Position: X: %d Y: %d", m_tmpElement->x(), m_tmpElement->y());

        m_dragPosOffset = m_tmpElement->pos() - event->pos();
    }

    //qCDebug(logC, "Debug state of element: %d", m_dragElement->getDebugState());


    //qCDebug(logC, "called - on child XYZ, Pos[X,Y]: %d, %d", child->pos().x(), child->pos().y());
    //qCDebug(log_workingarea, "called - on child XYZ, Pos[X,Y]: %d, %d", event->pos().x(), event->pos().y());



    update();
}

void WorkingArea::mouseReleaseEvent(QMouseEvent *event)
{
    //qCDebug(logC, "Element Position: X: %d Y: %d", m_dragElement->pos().x(), m_dragElement->pos().y());
    //qCDebug(logC, "Event Position: X: %d Y: %d", event->x(), event->y());
    //qCDebug(logC, "Size workingarea: X: %d Y: %d", width(), height());
    //qCDebug(logC, "Size workingarea: X: %d Y: %d", p);
    if(m_draw){
        QWidget *e = qobject_cast<QWidget*>(childAt(event->pos()));

        if (!e){
            qCDebug(logC, "called - no child");
            m_draw = false;
            update();
            return;
        }
        /*
         *  Hierarchy of ElementMaster
         *
         *  m_symbol(QLabel) --(parent)-->
         *                   m_symbolWidget(QWidget) --(parent)-->
         *                                           m_innerWidget(QLabel) --(parent)-->
         *                                                                 ElementMaster
         */
        ElementMaster* targetElement = qobject_cast<ElementMaster*>(e->parent()->parent()->parent());

        /* Position abfragen */

        if (!targetElement){
            qCDebug(logC, "no endpoint");
            m_draw = false;
            update();
            return;
        }

        if(helper::mouseOverElement(qobject_cast<QWidget*>(&(targetElement->m_socket)), event->globalPos())){
            qCDebug(logC, "Socket found - add Connection!");

            /*
             *  sender  = m_tmpElement
             *  receiver = targetElement
             */
            // QLine initialisieren?
            m_connections.append(Connection{m_tmpElement, targetElement, QLine()});

        }

        m_draw = false;
        update();

        //qCDebug(logC, "widget position: x: %d y: %d", withinSocketPos.x(), withinSocketPos.y());

    } else if (m_dragging){

        this->setCursor(Qt::ArrowCursor);
        /* Prevent that the element moves out of the
         * leftmost / topmost area */
        if(m_tmpElement->pos().x() < 0) m_tmpElement->move(0, m_tmpElement->y());
        if(m_tmpElement->pos().y() < 0) m_tmpElement->move(m_tmpElement->x(), 0);

        /* Resize the workingarea if the element was
         * moved out of the rightmost/bottommost initial size*/

        int max_x = 0;
        int max_y = 0;
        int new_x = MINIMUM_SIZE.width();
        int new_y = MINIMUM_SIZE.height();

        /* Get the left- and botmost element position */
        for(auto const &qobj : children()){

            ElementMaster* e = dynamic_cast<ElementMaster*>(qobj);

            max_x = e->pos().x() > max_x ? e->pos().x() : max_x;
            max_y = e->pos().y() > max_y ? e->pos().y() : max_y;

        }

        max_x += (m_tmpElement->width() / 2);
        max_y += (m_tmpElement->height() / 2);


        if( max_x < (width() + m_tmpElement->width()) &&
            max_x > MINIMUM_SIZE.width()){

            new_x = max_x + m_tmpElement->width();

        }

        if( max_y < (height() + m_tmpElement->height()) &&
            max_y > MINIMUM_SIZE.height()){

            new_y = max_y + m_tmpElement->height();
        }

        setMinimumSize(new_x, new_y);
        qCDebug(logC, "MaxX: %d MaxY: %d", max_x, max_y);
        qCDebug(logC, "Resize to X: %d Y: %d", width(), height());

        m_dragging = false;
    }
    m_tmpElement = NULL;
    update();
}

void WorkingArea::mouseMoveEvent(QMouseEvent *event)
{
    /*
     * Dragging an element
     *
     * Prevent to drag the element in the leftmost / topmost nirvana
     */
    if(     m_dragging &&
            event->x() > 0 &&
            event->y() > 0)
    {

        m_tmpElement->move(event->pos() += m_dragPosOffset);
        update();

    } else if (m_draw){

    /*
     * Draw connections
     */

    /* Draw preview */
    m_previewConnection = QLine(m_drawStartPos, event->pos());
    update();
     /*
      * Start & Stop highlighting the socket
      */

        /* Stop highlighting the socket */

        if( m_tmpElement &&
            m_mouseOverSocket &&
            !helper::mouseOverElement(qobject_cast<QWidget*>(&(m_tmpElement->m_socket)), event->globalPos()) )
        {
                QApplication::postEvent(&(m_tmpElement->m_socket), new QEvent(QEvent::Leave));
                m_mouseOverSocket = false;
        }

        /* Returns NULL if nothing is found */
        QWidget *e = qobject_cast<QWidget*>(childAt(event->pos()));

        if (!e){
            return;
        }
        /*
         *  Hierarchy of ElementMaster
         *
         *  m_symbol(QLabel) --(parent)-->
         *                   m_symbolWidget(QWidget) --(parent)-->
         *                                           m_innerWidget(QLabel) --(parent)-->
         *                                                                 ElementMaster
         */
        ElementMaster *elm = qobject_cast<ElementMaster*>(e->parent()->parent()->parent());

        if (!elm){
            return;
        }


        if( !m_mouseOverSocket &&
            helper::mouseOverElement(qobject_cast<QWidget*>(&(elm->m_socket)), event->globalPos())){

            /* Start highlighting the socket */
            QApplication::postEvent(&(elm->m_socket),
                                    new QEnterEvent(event->pos(),
                                                    event->windowPos(),
                                                    event->screenPos()));
            m_mouseOverSocket = true;
        }


    } // else if (m_draw)
}
#if 0
bool WorkingArea::mouseOverElement(const QWidget *element, const QPoint &globalPos)
{
    QPoint withinElementPos = element->mapFromGlobal(globalPos);

    return (withinElementPos.x() >= 0 &&
            withinElementPos.x() <= element->width() &&
            withinElementPos.y() >= 0 &&
            withinElementPos.y() <= element->height());
}
#endif
void WorkingArea::paintEvent(QPaintEvent *event)
{
    m_painter.begin(this);

    /* Reset background */

    m_backgroundGradient.setFinalStop(frameRect().bottomRight());
    QBrush brush = QBrush(m_backgroundGradient);
    m_painter.fillRect(frameRect(), brush);

    /* Draw connections */

    m_painter.setPen(m_pen);


    if(m_draw){

        drawPreviewConnection(&m_painter);
    }

    updateConnection();
    drawConnections(&m_painter);
    m_painter.end();
}




void WorkingArea::drawPreviewConnection(QPainter *p)
{

    p->drawLine(m_previewConnection);

    //p->drawLines(m_connections);
}

void WorkingArea::drawConnections(QPainter *p)
{

    for(const auto &pair : m_connections){

        p->drawLine(pair.connLine);
    }


}

void WorkingArea::updateConnection()
{
    for(auto &pair : m_connections){
        pair.connLine.setP1(pair.sender->pos() + PLUG_OFFSET_POSITION);
        pair.connLine.setP2(pair.receiver->pos() + SOCKET_OFFSET_POSITION);
    }
}

