#ifndef GUIBLOCK_LINK_H
#define GUIBLOCK_LINK_H

#include <memory>
#include <vector>
#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QPen>
#include "Block.h"

namespace GuiBlocks {

class Link : public QGraphicsItem
{
private: //internal types
    class LinkBinTree
    {
    friend Link;
    public:
        //types
        static constexpr uint16_t invalid_index = 0xFFFF;
        struct Node
        {
            Node() noexcept;
            Node(const QPointF &point,
                 uint16_t prevNode=invalid_index,
                 uint16_t firstChildIdx=invalid_index,
                 uint16_t parentNextChildIdx=invalid_index) noexcept;
            bool isEmpty() const noexcept;
            void makeEmpty() noexcept;
            void resetIndexs() noexcept;
            void setData(const QPointF &point,
                         uint16_t prevNode=invalid_index,
                         uint16_t firstChildIdx=invalid_index,
                         uint16_t parentNextChildIdx=invalid_index) noexcept;
            void show() noexcept;
            QPointF point;
            uint16_t firstChildIdx;
            uint16_t parentNextChildIdx;
            uint16_t prevNode;
        };//struct Node

        //ctors
        LinkBinTree(const QPointF& pos) noexcept;

        //node managment
        uint16_t appendChild(uint16_t parentIdx,const QPointF &point) noexcept;
        uint16_t insertBefore(uint16_t targetIdx,const QPointF &point) noexcept;
        void     removeSubTree(uint16_t targetIdx) noexcept;
        uint16_t childrenCount(uint16_t targetIdx) noexcept;
        uint16_t getParent(uint16_t targetIdx)const noexcept;
        QPointF &operator[](uint16_t idx);
        //this is a convinient method to be called instead of operator[] from a const method.
        //since operator[] returns a QPointF& it can not be marked as const, hence can not
        //be called from a const method
        const QPointF &getPoint(uint16_t idx) const;

        //general methods
        const QRectF& getContainerRect() const noexcept{ return containerRect; }
        uint16_t length() const noexcept;

        //iterators
        //this methods will iterate through all the nodes
        //(including startIdx) that are connected
        void resetSubTreeIterator(uint16_t startIdx) noexcept;
        uint16_t iterateSubTree() noexcept;
        //for public use:
        //The user can use multiple instances of IterPointers and
        //pass them to the resetIterator() and iterate()/iterateIdx()
        //as argument to iterate in parallel without conflicts
        class IterPointers
        {
        friend LinkBinTree;
        public:
            IterPointers(uint16_t startIdx=invalid_index):iterStart(startIdx){}
        private:
            uint16_t iterStart;
            uint16_t iterParent;
            uint16_t iterChild;
        };
        void resetIterator(IterPointers *pointers=nullptr) const noexcept;
        std::optional<std::tuple<const QPointF&,const QPointF&>>
        iterate(IterPointers *pointers=nullptr) const noexcept;
        //this behaves as iterate() but retorning the indexs of the points
        std::optional<std::tuple<uint16_t,uint16_t>>
        iterateIdx(IterPointers *pointers=nullptr) const noexcept;

        //debug
        void showNodes() const noexcept;
        void showIteration(uint16_t startIdx=invalid_index) noexcept;
        void showSubTreeIterations(uint16_t startIdx) noexcept;

    private://internal methods
        //iterator for internal use:
        //call first resetChildIter with a valid parentIdx, and then call
        //childIter to iterate over the parentIdx's childrens
        void resetChildIter(uint16_t parentIdx) noexcept;
        uint16_t childIter() noexcept;

        //return the first unused (empty) node after startIdx,
        //or invalid_index if none available
        uint16_t nextEmptyNode(uint16_t startIdx=0) const noexcept;
        //pick the first unused (empty) node, or creates a new one
        uint16_t createNode(const QPointF &pos,
                            uint16_t prevNode=invalid_index,
                            uint16_t firstChildIdx=invalid_index,
                            uint16_t parentNextChildIdx=invalid_index) noexcept;

        //this method removes targetIdx if it is located on the line
        //between prevNode and firstChildIdx of it, and if targetIdx
        //has only one child (ie, its a jointnode, see isJointNode method)
        bool simplifyAlignedNode(uint16_t targetIdx) noexcept;

        //helpers:
        //the next two methods are only to be used from iterator() method
        //(since does not check boundaries)
        bool isParent(uint16_t parentIdx,uint16_t childIdx) const noexcept;
        bool prevNodeIsParent(uint16_t childIdx) const noexcept;
        //if a node has more tha one child, it is considered as a jointnode
        bool isJointNode(uint16_t targetIdx) const noexcept;
        //if point is on the line joining two nodes, this method returns the
        //indexs of the two nodes. If point is exactly one node, will return
        //the node index as first output arg and invalid_index as the second
        std::optional<std::tuple<uint16_t,uint16_t>> isOnTrajectory(const QPointF &point) const noexcept;
        std::optional<std::tuple<uint16_t,uint16_t>> isOnTrajectory(const QRectF &rect) const noexcept;
        std::optional<std::tuple<uint16_t,uint16_t>> isMiddleOfLine(uint16_t targetIdx) const noexcept;
        bool isBetweenTwoNodes(uint16_t targetIdx) const noexcept;
        std::optional<std::tuple<uint16_t,uint16_t>> belongsToLine(uint16_t idxP1,
                                                                   uint16_t idxP2,
                                                                   const QPointF& point) const noexcept;

        //vars
        std::vector<Node> nodes;
        QRectF containerRect;
        uint16_t rootIdx;
        //iterator vars
        mutable IterPointers iterPointers;
        uint16_t iterSubTreeStart;
        uint16_t iterSubTreeIdx;
        uint16_t iterChildIdx;
    };  //class LinkBinTree  //class LinkBinTree
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
    //virtual ~Link() override {}

public: //pure virtual methods
    QRectF boundingRect() const override { return containerRect; }
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

public: //general methods
    bool isPartOfLink(const QPointF &point) const noexcept;
    bool isPartOfLink(const QRectF &rect) const noexcept;
    void insertLineAt(const QPointF &start,const QPointF &end,const LinkPath &linkPath) noexcept;
    void updateLastInsertedLine(const QPointF &end,const LinkPath &linkPath) noexcept;
    void simplifyLastInsertedLine() noexcept;
    void removeLastInsertedLine() noexcept;
    void selectArea(const QPainterPath &shape) noexcept;
    void selectAreaFirstItem(const QPainterPath &shape) noexcept;
    void selectAreaNearestItem(const QPointF &pos) noexcept;
    void displaceSelectedArea(const QPointF &offset) noexcept;
    void simplifySelectedArea() noexcept;
    bool isPosOnlyEndPoint(const QPointF &pos) noexcept;
    auto length()const noexcept{ return tree.length(); }
    bool isEmpty() const noexcept{ return tree.length()<=1; }
    void jointLink(Link& other);

    void showRawData() const noexcept;

    //port management
    void appendPort(const Block::Port *port);


private: //internal methods
    std::optional<QPointF> computeMidPoint(const QPointF  &startPoint,
                                           const QPointF  &endPoint,
                                           const LinkPath &linkPath) const noexcept;
    void updateContainerRect();
    bool simplifyRootNode() noexcept;
    std::optional<std::tuple<QPointF,double>> getTaxiDistanceAndPoint(const QPointF &pos);
    //this method will return the indexs of the two points of the line grabbed or
    //the index of the point grabbed (in the first element of the tuple, the second will be invalid_index)
    std::tuple<uint16_t,uint16_t> getGrabbedIndexs(const QPointF &pos) const noexcept;

private: //internal vars
    //indexs of the lines
    uint16_t idxStart = 0;
    uint16_t idxMid   = LinkBinTree::invalid_index;
    uint16_t idxEnd   = LinkBinTree::invalid_index;
    //stores the points of the link
    LinkBinTree tree;
    //indexes that will be loaded by selectArea() and then moved by moveSelection()
    std::vector<uint16_t> selectedIdx;
    //to store the multiples pasive ports
    std::vector<std::weak_ptr<Block::Port>> pasivePorts;
    //to store the only active port that can be connected to a link
    std::weak_ptr<Block::Port> activePort;
    QRectF containerRect;

private: //internals for debug porpouses
    struct
    {
        QPen pen = QPen(QBrush(StyleLink::normalColor),
                        qreal(StyleLink::width),
                        Qt::SolidLine,
                        Qt::RoundCap);
    }debug;

};
} // namespace GuiBlock

#endif // GUIBLOCK_LINK_H
