#include "ApplicationSetup.hpp"
#include "TestGraphModel.hpp"

#include <catch2/catch.hpp>

#include <QtNodes/internal/AbstractNodeGeometry.hpp>
#include <QtNodes/internal/BasicGraphicsScene.hpp>
#include <QtNodes/internal/DefaultNodePainter.hpp>
#include <QtNodes/internal/NodeGraphicsObject.hpp>
#include <QtNodes/internal/NodeStyle.hpp>

#include <QImage>
#include <QJsonObject>
#include <QPainter>

TEST_CASE("Lightweight shadows are opt-in and survive style serialization", "[graphics][shadows]")
{
    auto app = applicationSetup();

    QtNodes::NodeStyle legacyStyle;
    legacyStyle.ShadowEnabled = true;
    CHECK_FALSE(legacyStyle.FastShadowEnabled);

    auto legacyJson = legacyStyle.toJson();
    QJsonObject legacyNodeJson = legacyJson.value("NodeStyle").toObject();
    legacyNodeJson.remove("FastShadowEnabled");
    legacyJson["NodeStyle"] = legacyNodeJson;
    CHECK_FALSE(QtNodes::NodeStyle(legacyJson).FastShadowEnabled);

    QtNodes::NodeStyle fastStyle;
    fastStyle.ShadowEnabled = true;
    fastStyle.FastShadowEnabled = true;
    CHECK(QtNodes::NodeStyle(fastStyle.toJson()).FastShadowEnabled);

    TestGraphModel model;
    auto const legacyId = model.addNode("legacy");
    auto const fastId = model.addNode("fast");
    REQUIRE(model.setNodeData(legacyId, QtNodes::NodeRole::Style, legacyStyle.toJson()));
    REQUIRE(model.setNodeData(fastId, QtNodes::NodeRole::Style, fastStyle.toJson()));

    QtNodes::BasicGraphicsScene scene(model);
    auto *legacyNode = scene.nodeGraphicsObject(legacyId);
    auto *fastNode = scene.nodeGraphicsObject(fastId);
    REQUIRE(legacyNode != nullptr);
    REQUIRE(fastNode != nullptr);
    CHECK(legacyNode->graphicsEffect() != nullptr);
    CHECK(fastNode->graphicsEffect() == nullptr);
}

TEST_CASE("Lightweight shadow fits within default node margins", "[graphics][shadows]")
{
    auto app = applicationSetup();

    TestGraphModel model;
    auto const id = model.addNode("fast");

    QtNodes::NodeStyle style;
    style.ShadowEnabled = true;
    style.FastShadowEnabled = true;
    style.ShadowColor = QColor(0, 0, 0);
    REQUIRE(model.setNodeData(id, QtNodes::NodeRole::Style, style.toJson()));

    QtNodes::BasicGraphicsScene scene(model);
    auto *node = scene.nodeGraphicsObject(id);
    REQUIRE(node != nullptr);

    QSize const size = scene.nodeGeometry().size(id);
    REQUIRE(size.width() > 0);
    REQUIRE(size.height() > 0);

    // The default horizontal and vertical geometries reserve 20 px per side.
    QRectF const shadowFootprint(-6, -6, size.width() + 20, size.height() + 20);
    CHECK(scene.nodeGeometry().boundingRect(id).contains(shadowFootprint));

    QImage image(size.width() + 60, size.height() + 60, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.translate(25, 25);
    QtNodes::DefaultNodePainter nodePainter;
    nodePainter.drawNodeRect(&painter, *node);
    painter.end();

    int const midX = 25 + size.width() / 2;
    CHECK(qAlpha(image.pixel(midX, 25 + size.height() + 6)) > 0);
    CHECK(qAlpha(image.pixel(midX, 25 + size.height() + 19)) == 0);
}
