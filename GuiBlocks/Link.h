#ifndef GUIBLOCK_LINK_H
#define GUIBLOCK_LINK_H

#include <memory>
#include <vector>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include "Block.h"

namespace GuiBlocks {
#define LINK_V3
#ifdef LINK_V3
class Link : public QGraphicsItem
{
private: //internal types
    class LinkTree
    {
    public: //exported types & symbols
        static constexpr uint16_t invalid_index = 0xFFFF;
        struct IndexInfo
        {
            bool overTrajectory;
            uint16_t idx;   //if overTrajectory==true, the idx should be used with insertPointBefore
                            //to insert the point in the correct position
        };
    private: //internal types
        struct Node
        {
            QPointF point;
            uint16_t connCount;
        };

    public://ctors & dtors
        LinkTree(const QPointF& startPos,uint16_t size=3);

        //point managment
        uint16_t appendPoint(uint16_t parentPointIdx,const QPointF& pos);
        void     removePointAndSubTree(uint16_t idx);
        uint16_t insertPointBefore(uint16_t idx,const QPointF& pos);

        void     setPoint(uint16_t idx,const QPointF &point);
        QPointF  getPoint(uint16_t idx)const{return nodes[idx].point;}

        void     removeLastSubTree(uint16_t idx);
        uint16_t getParent(uint16_t idx);
        uint16_t getChild(uint16_t idx);
        //uint16_t getLastChild(uint16_t idx);
        std::optional<uint16_t> exists(const QPointF& point) const;
        std::optional<IndexInfo> isOnTrajectory(const QPointF &point);

        //geometry and size
        size_t length()const{ return nodes.size(); }
        QRectF getContainerRect()const{ return containerRect; }

        //iterator
        void resetIterator();
        std::tuple<const QPointF*,const QPointF*> iterateLine();
        std::tuple<const Node*,const Node*> iterateData();
        std::tuple<uint16_t,uint16_t> getIteratorIndex() const;

        void showRaw();
    private: //internal methods
        std::optional<uint16_t> findIdx(const QPointF& point) const;
        void updateContainerRect();
        void iterateToMatch(uint16_t &from,
                            uint16_t &to,
                            uint16_t target,
                            uint16_t &match,
                            bool resetPointers = true);
        //In the first call the third argument should be zero (default value)
        //for this recursive method to work properly
        void recursiveInsert(uint16_t idx,uint16_t childCount=1);
        void show();    //for debug, use qDebug()

    private: //deleted methods
        //When a point is modified the bounding rect should be updated,
        //and the operator[] should not be used for this, since the actual
        //modification to the point takes place in the the calling function
        //after the operator[] retunrs making imposible for the operator[]
        //to update the bounding rect. To avoid this issue the operator[]
        //is intentionaly deleted.
        QPointF &operator[](uint16_t idx) = delete;

    private: //internal types
        std::vector<Node> nodes;
        uint16_t iterFrom = 0;
        uint16_t iterTo   = 1;
        uint16_t iterCount = 0;
        IndexInfo cacheInfo;
        QPointF   cachePoint = QPointF(1.0e9,1.0e9);
        QRectF containerRect;
    };

public: //exported types
    enum class LinkPath
    {
        straight,               //direct line between start and end point
        verticalThenHorizontal,
        horizontalThenVertical,
        straightThenOrthogonal, //45 degs then horizontal or vertical
        orthogonalThenStraight  //horizontal or vertical then 45 degs
    };
public: //ctors & dtor
    Link(const QPointF &startPos);
    Link(const Link& l);
    virtual ~Link() override {}

public: //pure virtual methods
    QRectF boundingRect() const override { return points.getContainerRect(); }
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

public: //general methods
    bool containsPoint(const QPointF &point);
    void appendLine(//const QPointF  &startPoint,
                    const QPointF  &endPoint,
                    const LinkPath &linkPath);  //not implemented
    void insertLine(const QPointF &startPoint,
                    const QPointF &endPoint
                    ,const LinkPath &linkPath);
    void updateLastLine(const QPointF  &endPoint,
                        const LinkPath &linkPath);  //not implemented
    void joinLink(Link &&link,const QPointF& joinPoint);    //not implemented
    void show(){ points.showRaw(); }

private: //internal methods
    std::optional<QPointF> computeMidPoint(const QPointF  &startPoint,
                                           const QPointF  &endPoint,
                                           const LinkPath &linkPath);
private: //internal vars
    //indexs of the lines
    uint16_t startIdx = 0;
    uint16_t midIdx   = LinkTree::invalid_index;
    uint16_t endIdx   = LinkTree::invalid_index;
    QPointF firstLineEndPoint;
    //stores the points of the link
    LinkTree points;//replaces the VectorTree points; //replaces the old std::vector<QPointF> points;
    //to store the multiples pasive ports
    std::vector<std::weak_ptr<Block::Port*>> pasivePorts;
    //to store the only active port that can be connected to a link
    std::weak_ptr<Block::Port*> activePort;

private: //internals for debug porpouses
    struct
    {
        QPen pen = QPen(QBrush(StyleLink::normalColor),
                        qreal(StyleLink::width),
                        Qt::SolidLine,
                        Qt::RoundCap);
    }debug;

};
#elif defined(LINK_V2)
class Link : public QGraphicsItem
{
private: //types
    class LinkTree
    {
    public: //exported types & symbols
        static constexpr uint16_t invalid_index = 0xFFFF;
    public:
        LinkTree(const QPointF& startPos,uint16_t size=3);

        //point managment
        uint16_t addPoint(const QPointF& parentPoint,const QPointF& pos);
        uint16_t addPoint(uint16_t parentPointIdx,const QPointF& pos);
        void updatePoint(uint16_t idx,const QPointF &point);
        void removeLastSubTree(uint16_t idx);
        void removePoint(uint16_t idx);
        uint16_t getParent(uint16_t idx);
        QPointF operator[](uint16_t idx)const{return nodes[idx].point;}
        std::optional<uint16_t> exists(const QPointF& point) const;

        //geometry and size
        size_t length()const{ return nodes.size(); }
        QRectF getContainerRect()const{ return containerRect; }

        //iterator
        void resetIterator();
        std::tuple<const QPointF*,const QPointF*> iterateLine();
        uint16_t getIteratorIndex() const;

        struct Node
        {
            QPointF point;
            uint16_t connCount;
        };
    private: //internal methods
        std::vector<Node> nodes;
        std::optional<uint16_t> findIdx(const QPointF& point) const;
        void updateContainerRect();
        void iterateToMatch(uint16_t &from, uint16_t &to, uint16_t target, uint16_t &match, bool resetPointers = true);

    private: //internal types
        uint16_t fromIdx = 0;
        uint16_t toIdx   = 1;
        uint16_t iterCount = 0;
        QRectF containerRect;

    };
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


public: //ctors & dtor
    Link(const QPointF &startPos);
    Link(const Link& l);
    virtual ~Link() override {}

public: //pure virtual methods
    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

public: //general methods
//    //VectorTree related methods:
//    void appendPoint(const QPointF& point)      {points.append(point);prepareGeometryChange();update();}
//    void updateLastPoint(const QPointF& point)  {points.updateLast(point);prepareGeometryChange();update();}
//    bool containsPoint(const QPointF& point) const { return points.contains(point); }
//    size_t length() const { return points.length(); }
//    void getLastTwoIndexes(uint16_t &prev,uint16_t &last){ points.getLastTwoIndexes(prev,last); }
//    void deleteLastLine(){ points.deleteLastPoint();prepareGeometryChange();update(); }
//    void printSeq(){ points.printAllData(); }
//    void switchLinePath();

    //LinkTree related methods:
    size_t length()const{ return points.length(); }
    bool containsPoint(const QPointF &point)const{ return points.exists(point).has_value(); }
    void setActive(const QPointF &point);
    void updateActive(const QPointF &point);
    void updateActiveParent(const QPointF &point);
    uint16_t addPoint(const QPointF& point);
    void removePoint(const QPointF& point);
    QPointF operator[](uint16_t idx){return points[idx];}
    //Link::LinkPath getLinkPath()const{ return linkPath; }
//    std::tuple<uint16_t,uint16_t> getActiveAndParentIdx()const{ return {activePointIdx,activeParentIdx};}
    uint16_t getActiveIdx()const{ return activePointIdx;}
    uint16_t getActiveParentIdx()const{ return activeParentIdx;}

private: //internal methods

private: //internal vars
    uint16_t activePointIdx;
    uint16_t activeParentIdx;
    //stores the points of the link
    LinkTree points;//replaces the VectorTree points; //replaces the old std::vector<QPointF> points;
    //to store the multiples pasive ports
    std::vector<std::weak_ptr<Block::Port*>> pasivePorts;
    //to store the only active port that can be connected to a link
    std::weak_ptr<Block::Port*> activePort;
    //define the path that the active lines follows
    //LinkPath linkPath;

private: //internals for debug porpouses
    struct
    {
        QPen pen = QPen(QBrush(StyleLink::normalColor),
                        StyleLink::width,
                        Qt::SolidLine,
                        Qt::RoundCap);
    }debug;

};
#elif defined(LINK_V1)
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
