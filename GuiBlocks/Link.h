#ifndef GUIBLOCK_LINK_H
#define GUIBLOCK_LINK_H

#include <memory>
#include <QVector>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include "Block.h"
#include <vector>

namespace GuiBlocks {
#define LINK_V2
#ifdef LINK_V2
class Link : public QGraphicsItem
{
private: //types
    class VectorTree
    {
    private: //types
        static constexpr uint16_t invalid_index = 0xFFFF;

    public: //public interface
        VectorTree(uint16_t len);
        VectorTree(const VectorTree& v);
        //VectorTree(VectorTree&& v);
        ~VectorTree();
        void append(const QPointF& point);
        void updateLast(const QPointF& point);
        void getLastTwoIndexes(uint16_t &prev,uint16_t &last);
        QPointF getFirstPoint(){ return points[0]; }
        void deleteLastPoint();

        uint16_t length()     const { return pIdx; }
        uint16_t byteLength() const { return rawSize; }

        void optimizeMem();
        //void optimizeSequence();

        const QPointF* iterate();
        void resetIterator();

        QRectF getContainerRect()const{ return containerRect; }
        bool contains(const QPointF& point)const{ return find(point) != invalid_index; }

        uint16_t findIndex(const QPointF* point);
        void printAllData();
        void printNodes();
        void printEndpoints();

    private: //internal vars
        uint16_t activePointIdx;

        uint16_t rawSize;
        uint16_t pIdx;
        uint16_t sIdx;
        uint16_t pIter;
        uint16_t sIter;
        bool repeatPrev;
        QRectF containerRect;

        uint8_t *rawBuf;
        QPointF *points;
        struct Sequence
        {
            uint16_t idx;
            uint16_t count;
        }*sequence,seqIter;
    private: //internal methods
        Sequence* getSequence(uint16_t idx) const;
        void setSequence(const Sequence& sec);
        uint16_t find(const QPointF &point) const;
        bool setAsActiveIfExists(const QPointF &point);
        void expand();
        void updateContainerRect();
    };
public: //types
    enum class LinkPath
    {
        straight,               //direct line between start and end point
        verticalThenHorizontal,
        horizontalThenVertical,
        straightThenOrthogonal, //45 degs then horizontal or vertical
        orthogonalThenStraight  //horizontal or vertical then 45 degs
    };

public: //ctors & dtor
    Link(const QPointF &startPos,
         LinkPath linkPath = LinkPath::orthogonalThenStraight);
    Link(const Link& l);
    virtual ~Link() override {}

public: //pure virtual methods
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

public: //general methods
    void appendPoint(const QPointF& point)      {points.append(point);prepareGeometryChange();update();}
    void updateLastPoint(const QPointF& point)  {points.updateLast(point);prepareGeometryChange();update();}
    bool containsPoint(const QPointF& point) const { return points.contains(point); }
    uint16_t length() const { return points.length(); }
    void getLastTwoIndexes(uint16_t &prev,uint16_t &last){ points.getLastTwoIndexes(prev,last); }
    void deleteLastLine(){ points.deleteLastPoint();prepareGeometryChange();update(); }
    void printSeq(){ points.printAllData(); }

private: //internal methods

private: //internal vars
    //active point (index of the std::vector<QPointF> vector) that
    //is currently receiving the mouse events
    uint32_t activePointIndex = -1;
    //stores the points of the link
    VectorTree points; //replaces the old std::vector<QPointF> points;
    //to store the multiples pasive ports
    std::vector<std::weak_ptr<Block::Port*>> pasivePorts;
    //to store the only active port that can be connected to a link
    std::weak_ptr<Block::Port*> activePort;
    //define the path that the active lines follows
    LinkPath linkPath;

private: //internals for debug porpouses
    struct
    {
        QPen pen = QPen(QBrush(StyleLink::normalColor),
                        StyleLink::width,
                        Qt::SolidLine,
                        Qt::RoundCap);
    }debug;

};
#elif defined LINK_V1
class Link : public QGraphicsItem
{
public: //types
    enum class LinkPath
    {
        straight,               //direct line between start and end point
        verticalThenHorizontal,
        horizontalThenVertical,
        straightThenOrthogonal, //45 degs then horizontal or vertical
        orthogonalThenStraight  //horizontal or vertical then 45 degs
    };
    enum class Mode
    {
        move,
        append
    };

public: //ctors & dtor
    Link(const QPointF &pos);
    virtual ~Link() override;

public: //pure virtual methods
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

public: //general methods
    //updates the end pos of the last line of the link
    void updateLinkEndPos(const QPointF &pos);
    void enableMode(Mode mode=Mode::move);
    //updates the start pos of the first line of the link
//    void updateLinkStartPos(const QPointF &pos);
//    void setActivePort(const std::weak_ptr<Block::Port*>& port);
//    void appendPasivePort(const std::weak_ptr<Block::Port*>& port);
//    void updateBoundingRect();
    void switchLinePath();

    //User Interaction methods:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
    void hoverMoveEvent(QGraphicsSceneHoverEvent *event) override;

private: //internal methods
    //[remove]adds a line at the end of the link
    void setLineAttributes();
    //bool interceptLine(const QPointF& pos);
    //this method determines the point (which belong to the link)
    //that is closest to pos
    std::optional<std::tuple<QPointF,double>> getTaxiDistanceAndPoint(const QPointF& pos);

private:
    QRectF containerRect = QRectF(0,0,0,0);
    LinkPath linkPath = LinkPath::horizontalThenVertical;
    std::vector<QLineF*> lines;
    //to store the multiples pasive ports
    std::vector<std::weak_ptr<Block::Port*>> pasivePorts;
    //to store the only active port that can be connected to a link
    std::weak_ptr<Block::Port*> activePort;

    QPointF lastEndPos;
    LinkPath lastLinkPath = LinkPath::horizontalThenVertical;
    QPainterPath shapePath;
    Mode mode = Mode::move;
    QPointF activePoint;
    struct Debug
    {
        QPen pen = QPen(QBrush(StyleLink::normalColor),
                        StyleLink::width,
                        Qt::SolidLine,
                        Qt::RoundCap);
        QRectF rect;
        QPointF nearestPoint;
        QLineF line;
        std::vector<QLineF> lines;
        bool toggleLineColor;
        bool first = true;
        std::vector<QPointF> points;
        void genPoints(int n);
    }debug;
};

#elif defined LINK_V2
class Link : public QGraphicsItem
{
private:
    QPainterPath path;
    QPointF start,end;
    std::weak_ptr<Block::Port> inPort;
    QVector<std::weak_ptr<Block::Port>> outPorts;
public:
    Link();
    void setStartPoint(const QPointF &startPos){ start = startPos; }
    void setEndPoint(const QPointF &endPos){ end = endPos; update(); }
    void setInPort(std::shared_ptr<Block::Port> port);
    void addOutPort(std::shared_ptr<Block::Port> port);
    void removeInPort(std::shared_ptr<Block::Port> port);
    void removeOutPort(std::shared_ptr<Block::Port> port);

    const QPainterPath& getPath(){ return path; }

    void mouseMoveEvent(QGraphicsSceneMouseEvent *event) override;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;
    QRectF boundingRect() const override { return path.boundingRect(); }

};
#endif
} // namespace GuiBlock

#endif // GUIBLOCK_LINK_H
