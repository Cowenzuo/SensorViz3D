//
// Created by chudonghao on 2023/7/25.
//

#ifndef OPEDITOR_CAMERAMANIPULATOR_H
#define OPEDITOR_CAMERAMANIPULATOR_H

#include <osgGA/GUIEventHandler>
#include <osgGA/TrackballManipulator>


#define assert(expression) ((void)0)

template <typename genType>
inline constexpr genType zero() {
	return genType(0);
}

template <typename genType>
inline constexpr genType one() {
	return genType(1);
}

template <typename genType>
inline genType quarticEaseOut(genType const& a) {
	// Only defined in [0, 1]
	assert(a >= zero<genType>());
	assert(a <= one<genType>());

	genType const f = (a - one<genType>());
	return f * f * f * (one<genType>() - a) + one<genType>();
}

/// \brief 基于点的相机操作器
/// 平移、旋转、拉近拉远默认基于鼠标位置处与场景的交点
/// 如果没有交点，则会基于内部规则获取一个点
class PointBasedManipulator : public osgGA::OrbitManipulator {
	typedef OrbitManipulator inherited;

public:
	enum class ViewMode {
		kPerspective,
		kOrthographic
	};

	PointBasedManipulator(int flags = DEFAULT_SETTINGS);

	/// 切换拉近拉远方向
	void ToggleZoomDirection();

	/// 设置旋转系数
	void SetRotationFactor(double factor);

	void SetTargetRotation(const osg::Quat& rotation, double duration);

	void SetTargetTranslation(const osg::Vec3f& center, double distance, double duration);

	void SetViewMode(ViewMode mode);

	void home(double) override;

protected:
	bool handleMousePush(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) override;

	bool handleMouseWheel(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) override;

	bool performMovementLeftMouseButton(const double eventTimeDelta, const double dx, const double dy) override;

	bool performMovementMiddleMouseButton(const double eventTimeDelta, const double dx, const double dy) override;

	bool performMovementRightMouseButton(const double eventTimeDelta, const double dx, const double dy) override;

	bool handleMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) override;

	void updateCamera(osg::Camera& camera) override;

	void home(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) override;

private:
	bool ZoomModel(const osg::Vec3d& basePoint, const float dy, bool pushForwardIfNeeded);

	bool GetIntersection(const osg::Plane& plane, const osg::Vec3d& lineStart, const osg::Vec3d& lineEnd, osg::Vec3d& intersection) const;

	bool GetIntersection(const osgGA::GUIEventAdapter& ea, osg::Vec3d& point) const;

	bool GetIntersection(const osg::Vec3d& start, const osg::Vec3d& end, osg::Vec3d& intersection) const;

	bool GetBasePoint(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us, osg::Vec3d& basePoint) const;

	bool AdjustCamera();

private:
	bool _mouse_pressed{ false };
	osg::Vec3d _mouse_press_intersection;
	osg::Vec3d _mouse_press_eye_pos;
	osg::Matrixd _mouse_press_vpw;
	osg::Vec3d _mouse_press_center;

	double _rotation_factor{ 1.5 };
	bool _inverse_zoom{ true };

	osg::Quat _start_rotation;
	osg::Quat _target_rotation;
	osg::Timer_t _rotate_start_time;
	double _rotate_duration{ 0.2 };
	bool _rotate_requested{};

	osg::Vec3f _start_center;
	osg::Vec3f _target_center;
	double _start_distance;
	double _target_distance;
	osg::Timer_t _translate_start_time;
	double _translate_duration{ 0.2 };
	bool _translate_requested{};

	ViewMode _view_mode{ ViewMode::kPerspective };
	double _ortho_projection_scale{ 1.0 };
	double _ortho_projection_distance{ 1.0 };
	double _ortho_height{ 1.0 };
	bool _projection_matrix_update_requested{ true };

	bool _switch_to_perspective_when_drag_rotate{ true };
};

class CameraManipulator : public PointBasedManipulator {
	using Super = osgGA::CameraManipulator;

protected:
	bool _drag_first{ false };

	osgGA::GUIEventAdapter::ModKeyMask _drag_mod_key{ static_cast<osgGA::GUIEventAdapter::ModKeyMask>(0) };
	osgGA::GUIEventAdapter::MouseButtonMask _drag_mouse{ osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON };

	osgGA::GUIEventAdapter::ModKeyMask _rotate_mod_key{ static_cast<osgGA::GUIEventAdapter::ModKeyMask>(0) };
	osgGA::GUIEventAdapter::MouseButtonMask _rotate_mouse{ osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON };

public:
	explicit CameraManipulator(int flags = DEFAULT_SETTINGS);

	void SetDragFirst(bool drag_first) {
		_drag_first = drag_first;
	}

	void SetDragModKeyMask(osgGA::GUIEventAdapter::ModKeyMask key) {
		_drag_mod_key = key;
	}

	void SetDragMouse(osgGA::GUIEventAdapter::MouseButtonMask drag_mouse) {
		_drag_mouse = drag_mouse;
	}

	void SetRotateModKeyMask(osgGA::GUIEventAdapter::ModKeyMask key) {
		_rotate_mod_key = key;
	}

	void SetRotateMouse(osgGA::GUIEventAdapter::MouseButtonMask rotate_mouse) {
		_rotate_mouse = rotate_mouse;
	}

	/// \param event_time_delta 时间间隔(s)
	/// \param dx 鼠标移动的X距离(0~1)
	/// \param dy 鼠标移动的Y距离(0~1)
	bool PerformMovementDrag(const double event_time_delta, const double dx, const double dy) {
		// middle mouse as drag
		return this->performMovementMiddleMouseButton(event_time_delta, dx, dy);
	}
	/// \param event_time_delta 时间间隔(s)
	/// \param dy 鼠标移动的距离(0~1)
	bool PerformMovementZoom(const double event_time_delta, const double, const double dy) {
		// right mouse as zoom
		return this->performMovementRightMouseButton(event_time_delta, 0, dy);
	}

	/// \param event_time_delta 时间间隔(s)
	/// \param dx 鼠标移动的X距离(0~1)
	/// \param dy 鼠标移动的Y距离(0~1)
	bool PerformMovementRotate(const double event_time_delta, const double dx, const double dy) {
		// left mouse as rotate
		return this->performMovementLeftMouseButton(event_time_delta, dx, dy);
	}

protected:
	bool performMovement() override;
};
#endif  // OPEDITOR_CAMERAMANIPULATOR_H
