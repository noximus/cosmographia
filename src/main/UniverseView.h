// This file is part of Cosmographia.
//
// Copyright (C) 2010 Chris Laurel <claurel@gmail.com>
//
// Eigen is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// Cosmographia is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Cosmographia. If not, see <http://www.gnu.org/licenses/>.

#ifndef _UNIVERSE_VIEW_H_
#define _UNIVERSE_VIEW_H_

#include "NetworkTextureLoader.h"
#include "catalog/UniverseCatalog.h"
#include <QDeclarativeView>
#include <QTimer>
#include <QDateTime>
#include <QGestureEvent>
#include <vesta/Universe.h>
#include <vesta/Observer.h>
#include <vesta/TextureMapLoader.h>
#include <vesta/MeshGeometry.h>
#include <vesta/Visualizer.h>
#include <vesta/TiledMap.h>

class QVideoEncoder;
class ObserverAction;
class Viewpoint;

class QGraphicsScene;

namespace vesta
{
    class UniverseRenderer;
    class TextureFont;
    class ConeGeometry;
    class Atmosphere;
    class CubeMapFramebuffer;
    class ObserverController;
    class Trajectory;
    class TrajectoryPlotGenerator;
    class GlareOverlay;
}

class UniverseView : public QDeclarativeView
{
     Q_OBJECT

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    UniverseView(QWidget *parent, vesta::Universe* universe, UniverseCatalog* catalog);
    ~UniverseView();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    bool isPaused() const
    {
        return m_paused;
    }

    double timeScale() const
    {
        return m_timeScale;
    }

    double simulationTime() const
    {
        return m_simulationTime;
    }

    void startVideoRecording(QVideoEncoder* encoder);
    void finishVideoRecording();
    bool isRecordingVideo() const
    {
        return m_videoEncoder != NULL;
    }

    QVideoEncoder* videoEncoder() const
    {
        return m_videoEncoder;
    }

    vesta::Universe* universe() const
    {
        return m_universe.ptr();
    }

    vesta::TextureMapLoader* textureLoader() const
    {
        return m_textureLoader.ptr();
    }

    vesta::Entity* selectedBody() const
    {
        return m_selectedBody.ptr();
    }

    enum TimeDisplayMode
    {
        TimeDisplay_UTC       = 0,
        TimeDisplay_Local     = 1,
        TimeDisplay_Multiple  = 2
    };

    TimeDisplayMode timeDisplay() const
    {
        return m_timeDisplay;
    }

    enum StereoMode
    {
        Mono               = 0,
        SideBySide         = 1,
        AnaglyphRedCyan    = 2,
        AnaglyphCyanRed    = 3
    };

    StereoMode stereoMode() const
    {
        return m_stereoMode;
    }

    void setSelectedBody(vesta::Entity* body);
    QImage grabFrameBuffer(bool withAlpha = false);

    void replaceEntity(vesta::Entity* entity, const BodyInfo* info);

    void initializeGL();


public slots:
    void tick();
    void setPaused(bool paused);
    void setCurrentTime();
    void setTimeScale(double scale);
    void setSimulationTime(double tsec);
    void inertialObserver(bool checked);
    void bodyFixedObserver(bool checked);
    void synodicObserver(bool checked);
    void setObserverCenter();
    void setMilkyWayVisibility(bool checked);
    void setEquatorialGridVisibility(bool checked);
    void setEclipticVisibility(bool checked);
    void setEquatorialPlaneVisibility(bool checked);
    void setPlanetographicGridVisibility(bool checked);
    void setConstellationFigureVisibility(bool checked);
    void setConstellationNameVisibility(bool checked);
    void setLabelVisibility(bool enable);
    void setShadows(bool enable);
    void setEclipseShadows(bool enable);
    void setCloudLayers(bool enable);
    void setAtmospheres(bool enable);
    void setAmbientLight(bool enable);
    void setReflections(bool enable);
    void setStereoMode(StereoMode stereoMode);
    void setSunGlare(bool enable);
    void setInfoText(bool enable);
    void plotTrajectory(vesta::Entity* body, const BodyInfo* info);
    void plotTrajectoryObserver(const BodyInfo* info);
    void clearTrajectory(vesta::Entity* body);
    void gotoSelectedObject();
    void setViewpoint(Viewpoint* viewpoint);
    void setTimeDisplay(TimeDisplayMode mode);

    void setUpdateInterval(unsigned int msec);
    void findObject();

private slots:
    void setSelectedBody(const QString& name);

protected:
    void paintGL();
    void resizeGL(int width, int height);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent* event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void contextMenuEvent(QContextMenuEvent* event);
    void paintEvent(QPaintEvent* event);
    bool event(QEvent* event);

private:
    void drawInfoOverlay();

    enum FrameType
    {
        Frame_Inertial,
        Frame_BodyFixed,
        Frame_Synodic,
    };

    void setCenterAndFrame(vesta::Entity* center, FrameType f);
    void initializeSkyLayers();
    void initializeObserver();
    double secondsFromBaseTime() const;
    vesta::TextureMap* loadTexture(const QString& location, const vesta::TextureProperties& texProps);

    void initNetwork();
    bool initPlanetEphemeris();

    void updateTrajectoryPlots();
    bool gestureEvent(QGestureEvent* event);

    vesta::Entity* pickObject(const QPoint& point);

private:
    int m_mouseMovement;
    QPoint m_mouseDownPosition;
    QPoint m_lastMousePosition;
    double m_lastDoubleClickTime;
    vesta::counted_ptr<vesta::Universe> m_universe;
    UniverseCatalog* m_catalog;
    vesta::counted_ptr<vesta::Observer> m_observer;
    vesta::counted_ptr<vesta::ObserverController> m_controller;
    vesta::UniverseRenderer* m_renderer;
    vesta::counted_ptr<vesta::GlareOverlay> m_glareOverlay;
    FrameType m_observerFrame;
    double m_fovY;

    bool m_rollLeft;
    bool m_rollRight;
    bool m_pitchDown;
    bool m_pitchUp;

    QTimer* m_timer;
    double m_realTime;
    double m_simulationTime;

    QDateTime m_baseTime;
    bool m_firstTick;
    double m_lastTickTime;

    double m_timeScale;
    bool m_paused;

    vesta::counted_ptr<vesta::TextureFont> m_titleFont;
    vesta::counted_ptr<vesta::TextureFont> m_textFont;
    vesta::counted_ptr<vesta::TextureFont> m_labelFont;
    vesta::counted_ptr<vesta::TextureMap> m_spacecraftIcon;

    unsigned int m_frameCount;
    double m_frameCountStartTime;
    double m_framesPerSecond;

    vesta::counted_ptr<vesta::Entity> m_selectedBody;

    vesta::counted_ptr<NetworkTextureLoader> m_textureLoader;
    vesta::counted_ptr<vesta::CubeMapFramebuffer> m_reflectionMap;
    vesta::counted_ptr<vesta::MeshGeometry> m_defaultSpacecraftMesh;

    bool m_reflectionsEnabled;
    StereoMode m_stereoMode;
    bool m_sunGlareEnabled;

    struct TrajectoryPlotEntry
    {
        TrajectoryPlotEntry();
        vesta::counted_ptr<vesta::Visualizer> visualizer;
        vesta::counted_ptr<vesta::Trajectory> trajectory;
        vesta::TrajectoryPlotGenerator* generator;
        unsigned int sampleCount;
        double leadDuration;
    };
    std::vector<TrajectoryPlotEntry> m_trajectoryPlots;

    bool m_infoTextVisible;
    bool m_labelsVisible;

    vesta::counted_ptr<ObserverAction> m_observerAction;

    QGraphicsScene* m_guiScene;

    QVideoEncoder* m_videoEncoder;
    TimeDisplayMode m_timeDisplay;
    bool m_wireframe;
};

#endif // _UNIVERSE_VIEW_H_
