//
// Created by chudonghao on 2023/7/25.
//

#include "CameraManipulator.h"

#include <osgUtil/LineSegmentIntersector>
#include <osgViewer/Viewer>

#include "ComputeIntersection.h"


template <typename T>
T lerp(const T& start, const T& end, float t) {
	return start + (end - start) * t;
}

PointBasedManipulator::PointBasedManipulator(int flags) : osgGA::OrbitManipulator(flags) {
	// 屏蔽惯性效果
	_allowThrow = false;
}

void PointBasedManipulator::ToggleZoomDirection() {
	_inverse_zoom = !_inverse_zoom;
}

void PointBasedManipulator::SetRotationFactor(double factor) {
	_rotation_factor = factor;
}

void PointBasedManipulator::SetTargetRotation(const osg::Quat& rotation, double duration) {
	_start_rotation = getRotation();
	_target_rotation = rotation;
	_rotate_duration = duration;
	_rotate_start_time = osg::Timer::instance()->tick();
	_rotate_requested = true;
}

void PointBasedManipulator::SetTargetTranslation(const osg::Vec3f& center, double distance, double duration) {
	_start_center = getCenter();
	_target_center = center;
	_start_distance = getDistance();
	_target_distance = distance;
	_translate_duration = duration;
	_translate_start_time = osg::Timer::instance()->tick();
	_translate_requested = true;
}

void PointBasedManipulator::SetViewMode(ViewMode mode) {
	if (_view_mode == mode) {
		return;
	}
	switch (mode) {
	case ViewMode::kPerspective: {
		_distance = _ortho_projection_distance / _ortho_projection_scale;
		_projection_matrix_update_requested = true;
		break;
	}
	case ViewMode::kOrthographic: {
		if (!getNode()) {
			return;
		}
		osg::BoundingSphere bounding_sphere = getNode()->getBound();
		_ortho_projection_distance = bounding_sphere.radius() * 2.0;
		_ortho_projection_scale = _ortho_projection_distance / _distance;
		_distance = 1000000.0;

		_projection_matrix_update_requested = true;

		break;
	}
	}
	_view_mode = mode;
}

void PointBasedManipulator::home(double x) {
	SetViewMode(ViewMode::kPerspective);
	OrbitManipulator::home(x);
}

bool PointBasedManipulator::handleMousePush(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) {
	osg::View* view = dynamic_cast<osg::View*>(&us);
	if (!view)
		return false;

	osg::Camera* camera = view->getCamera();
	if (!camera)
		return false;

	// 平移、旋转、拉近拉远都是基于鼠标与地面的交点进行的（如果存在的话）
	_mouse_pressed = true;
	_mouse_press_eye_pos = _center - _rotation * osg::Vec3d(0.0, 0.0, -_distance);
	_mouse_press_center = _center;
	osg::Matrix vpw = camera->getViewMatrix() * camera->getProjectionMatrix() * camera->getViewport()->computeWindowMatrix();
	_mouse_press_vpw = osg::Matrix::inverse(vpw);
	GetBasePoint(ea, us, _mouse_press_intersection);

	return inherited::handleMousePush(ea, us);
}

bool PointBasedManipulator::handleMouseWheel(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) {
	_thrown = false;

	osg::Vec3d point;
	if (!GetBasePoint(ea, us, point)) {
		return inherited::handleMouseWheel(ea, us);
	}

	osgGA::GUIEventAdapter::ScrollingMotion sm = ea.getScrollingMotion();
	switch (sm) {
	case osgGA::GUIEventAdapter::SCROLL_UP: {
		ZoomModel(point, -_wheelZoomFactor, false);
		us.requestRedraw();
		us.requestContinuousUpdate(isAnimating() || _thrown);
		return true;
	}
	case osgGA::GUIEventAdapter::SCROLL_DOWN: {
		ZoomModel(point, _wheelZoomFactor, false);
		us.requestRedraw();
		us.requestContinuousUpdate(isAnimating() || _thrown);
		return true;
	}
	default:
		break;
	}

	return false;
}

bool PointBasedManipulator::performMovementLeftMouseButton(const double eventTimeDelta, const double dx, const double dy) {
	if (_view_mode == ViewMode::kOrthographic && _switch_to_perspective_when_drag_rotate) {
		SetViewMode(ViewMode::kPerspective);
	}

	if (!_mouse_pressed) {
		return inherited::performMovementLeftMouseButton(eventTimeDelta, dx * _rotation_factor, dy * _rotation_factor);
	}

	osg::CoordinateFrame coordinateFrame = getCoordinateFrame(_center);
	osg::Vec3d localUp = getUpVector(coordinateFrame);

	osg::Quat rotation = _rotation;
	rotateYawPitch(_rotation, dx * _rotation_factor, dy * _rotation_factor, localUp);

	osg::Vec3d vector_center_to_intersect = _center - _mouse_press_intersection;
	osg::Quat rotation_delta = rotation.inverse() * _rotation;
	osg::Vec3d rotatedVectorAB = rotation_delta * vector_center_to_intersect;
	_center = _mouse_press_intersection + rotatedVectorAB;
	return true;
}

bool PointBasedManipulator::performMovementMiddleMouseButton(const double eventTimeDelta, const double dx, const double dy) {
	if (_thrown) {
		float scale = -0.3f * _distance * getThrowScale(eventTimeDelta);
		panModel(dx * scale, dy * scale);
	}
	else {
		if (!_mouse_pressed) {
			return true;
		}

		// 假设拖动场景对象，计算场景对象的平移值，再反过来移动相机观察的中心点
		osg::Plane plane(_mouse_press_eye_pos - _mouse_press_center, _mouse_press_intersection);
		plane.makeUnitLength();
		osg::Vec3d current_mouse_position = osg::Vec3d(_ga_t0->getX(), _ga_t0->getY(), 0) * _mouse_press_vpw;
		osg::Vec3d intersection;
		osg::Vec3d line_start;

		switch (_view_mode) {
		case ViewMode::kPerspective:
			line_start = _mouse_press_eye_pos;
			break;
		case ViewMode::kOrthographic:
			line_start = current_mouse_position - (_mouse_press_eye_pos - _mouse_press_center);
			break;
		}

		if (!GetIntersection(plane, line_start, current_mouse_position, intersection)) {
			return false;
		}

		osg::Vec3d offset = intersection - _mouse_press_intersection;

		_center = _mouse_press_center - offset;
	}
	return true;
}

bool PointBasedManipulator::performMovementRightMouseButton(const double eventTimeDelta, const double dx, const double dy) {
	zoomModel(dy * getThrowScale(eventTimeDelta), false);
	AdjustCamera();
	return true;
}

bool PointBasedManipulator::handleMouseRelease(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) {
	_mouse_pressed = false;
	return inherited::handleMouseRelease(ea, us);
}

void PointBasedManipulator::updateCamera(osg::Camera& camera) {
	if (_rotate_requested) {
		if (_rotate_duration <= 0.0) {
			setRotation(_target_rotation);
			_rotate_requested = false;
		}
		else {
			double elapsed_time = osg::Timer::instance()->delta_s(_rotate_start_time, osg::Timer::instance()->tick());

			double t = elapsed_time / _rotate_duration;
			if (t >= 1.0) {
				t = 1.0;
				_rotate_requested = false;
			}

			osg::Quat interpolatedRotation;
			interpolatedRotation.slerp(t, _start_rotation, _target_rotation);
			setRotation(interpolatedRotation);
		}
	}

	if (_translate_requested) {
		if (_translate_duration <= 0.0) {
			setCenter(_target_center);
			setDistance(_target_distance);
			_translate_requested = false;
		}
		else {
			double elapsed_time = osg::Timer::instance()->delta_s(_translate_start_time, osg::Timer::instance()->tick());

			double t = elapsed_time / _translate_duration;
			if (t >= 1.0) {
				t = 1.0;
				_translate_requested = false;
			}

			double factor = quarticEaseOut(t);
			osg::Vec3f interpolated_center = lerp(_start_center, _target_center, factor);
			double interpolated_distance = lerp(_start_distance, _target_distance, factor);
			setCenter(interpolated_center);
			setDistance(interpolated_distance);
		}
	}

	switch (_view_mode) {
	case ViewMode::kPerspective: {
		if (_projection_matrix_update_requested) {
			osg::Viewport* viewport = camera.getViewport();
			const double w = viewport->width();
			const double h = viewport->height();
			camera.setProjectionMatrixAsPerspective(60.0, w / h, 0.1, 1000);
			_projection_matrix_update_requested = false;
		}
		break;
	}
	case ViewMode::kOrthographic: {
		if (_projection_matrix_update_requested) {
			double fovy, aspect_ratio, z_near, z_far;
			if (camera.getProjectionMatrixAsPerspective(fovy, aspect_ratio, z_near, z_far)) {
				_ortho_height = 2.0 * _ortho_projection_distance * tan(osg::DegreesToRadians(fovy / 2.0));
			}

			osg::Viewport* viewport = camera.getViewport();
			const double w = viewport->width();
			const double h = viewport->height();
			double ratio = w / h;
			double bottom = -_ortho_height / _ortho_projection_scale / 2.0;
			double top = _ortho_height / _ortho_projection_scale / 2.0;
			double left = bottom * ratio;
			double right = top * ratio;
			camera.setProjectionMatrixAsOrtho(left, right, bottom, top, 0.1, 1000);

			_projection_matrix_update_requested = false;
		}
		break;
	}
	}

	OrbitManipulator::updateCamera(camera);
}

void PointBasedManipulator::home(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us) {
	SetViewMode(ViewMode::kPerspective);
	OrbitManipulator::home(ea, us);
}

bool PointBasedManipulator::ZoomModel(const osg::Vec3d& basePoint, const float dy, bool pushForwardIfNeeded) {
	double distance = _distance;

	switch (_view_mode) {
	case ViewMode::kPerspective: {
		// 相机到基准点的方向向量
		osg::Vec3d eye = _center - _rotation * osg::Vec3d(0.0, 0.0, -_distance);
		osg::Vec3d eye_to_mouse_direction = basePoint - eye;
		eye_to_mouse_direction.normalize();

		// 相机到中心点的方向向量
		osg::Matrixd rotation_matrix;
		rotation_matrix.makeRotate(_rotation);
		osg::Vec3d look_direction(-rotation_matrix(2, 0), -rotation_matrix(2, 1), -rotation_matrix(2, 2));

		zoomModel(dy, false);

		// zoomModel改变了_distance，需要重新计算_center
		// 向量点积
		double res_dot = eye_to_mouse_direction.x() * look_direction.x() + eye_to_mouse_direction.y() * look_direction.y() + eye_to_mouse_direction.z() * look_direction.z();
		double distance_delta = distance - _distance;
		double diff = (distance - _distance) / res_dot;                         // 沿鼠标点方向移动的距离
		osg::Vec3d new_eye = eye + eye_to_mouse_direction * diff;               // 沿鼠标点方向移动后的点，作为相机新的位置
		osg::Vec3d pos_towards_center = eye + look_direction * distance_delta;  // 沿中心方向移动后的点
		osg::Vec3d pan = new_eye - pos_towards_center;

		_center += pan;

		AdjustCamera();

		break;
	}
	case ViewMode::kOrthographic: {
		float scale = 1.0f - dy;
		_ortho_projection_scale *= scale;
		osg::Vec3d mouse_to_center = (basePoint - _center) / scale;
		_center = basePoint - mouse_to_center;

		_projection_matrix_update_requested = true;

		break;
	}
	}

	return true;
}

bool PointBasedManipulator::GetIntersection(const osg::Plane& plane, const osg::Vec3d& lineStart, const osg::Vec3d& lineEnd, osg::Vec3d& intersection) const {
	osg::Vec3d dir = lineEnd - lineStart;
	dir.normalize();

	double denominator = plane.getNormal() * dir;  // 平面法向量与线方向向量的点积

	// 检查分母是否为零，即线与平面是否平行
	if (fabs(denominator) < 1e-6) {
		return false;  // 无交点或线在平面内
	}

	double t = -(plane.getNormal() * lineStart + plane[3]) / denominator;

	intersection = lineStart + dir * t;

	return true;  // 成功计算交点
}

bool PointBasedManipulator::GetIntersection(const osgGA::GUIEventAdapter& ea, osg::Vec3d& point) const {
	osgUtil::LineSegmentIntersector::Intersections intersections;
	if (computeIntersections(ea, intersections, std::numeric_limits<osg::Node::NodeMask>::max())) {
		point = intersections.begin()->getWorldIntersectPoint();
		return true;
	}
	return false;
}

bool PointBasedManipulator::GetIntersection(const osg::Vec3d& start, const osg::Vec3d& end, osg::Vec3d& intersection) const {
	osg::ref_ptr<osgUtil::LineSegmentIntersector> lsi = new osgUtil::LineSegmentIntersector(start, end);

	osgUtil::IntersectionVisitor iv(lsi.get());
	iv.setTraversalMask(_intersectTraversalMask);

	_node->accept(iv);

	if (lsi->containsIntersections()) {
		intersection = lsi->getIntersections().begin()->getWorldIntersectPoint();
		return true;
	}
	return false;
}

bool PointBasedManipulator::GetBasePoint(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us, osg::Vec3d& basePoint) const {
	if (!GetIntersection(ea, basePoint)) {
		osg::View* view = dynamic_cast<osg::View*>(&us);
		if (!view)
			return false;

		osg::Camera* camera = view->getCamera();
		if (!camera)
			return false;

		osg::Matrixd rotation_matrix;
		rotation_matrix.makeRotate(_rotation);
		osg::Vec3d look_direction(-rotation_matrix(2, 0), -rotation_matrix(2, 1), -rotation_matrix(2, 2));
		osg::Plane plane(look_direction, _center);
		plane.makeUnitLength();

		osg::Matrix vpw = camera->getViewMatrix() * camera->getProjectionMatrix() * camera->getViewport()->computeWindowMatrix();
		osg::Vec3d current_mouse_position = osg::Vec3d(ea.getX(), ea.getY(), 0) * osg::Matrix::inverse(vpw);
		osg::Vec3d eye = _center - _rotation * osg::Vec3d(0.0, 0.0, -_distance);

		osg::Vec3d line_start;
		if (_view_mode == ViewMode::kOrthographic) {
			line_start = current_mouse_position - (eye - _center);
		}
		else {
			line_start = eye;
		}
		if (!GetIntersection(plane, line_start, current_mouse_position, basePoint)) {
			return false;
		}
	}
	return true;
}

bool PointBasedManipulator::AdjustCamera() {
	if (!_node) {
		return false;
	}

	// 调整相机的_center和_distance参数，使得相机可拉近到场景对象表面上，且不会穿透场景对象
	osg::Vec3d eye = _center - _rotation * osg::Vec3d(0.0, 0.0, -_distance);
	osg::Vec3d look_vector = _center - eye;
	look_vector.normalize();

	const osg::BoundingSphere& bs = _node->getBound();
	float distance = (eye - bs.center()).length() + _node->getBound().radius();
	osg::Vec3d start_segment = eye;
	osg::Vec3d end_segment = eye + look_vector * distance;

	osg::Vec3d ip;
	if (GetIntersection(start_segment, end_segment, ip)) {
		_center = ip;
		_distance = (eye - ip).length();

		if (_view_mode == ViewMode::kOrthographic) {
			_projection_matrix_update_requested = true;
		}

		return true;
	}

	return false;
}

CameraManipulator::CameraManipulator(int flags) : PointBasedManipulator(flags) {
}

// \see osgGA::StandardManipulator::performMovement()
bool CameraManipulator::performMovement() {
	// return if less then two events have been added
	if (_ga_t0.get() == NULL || _ga_t1.get() == NULL)
		return false;

	// get delta time
	double event_time_delta = _ga_t0->getTime() - _ga_t1->getTime();
	if (event_time_delta < 0.) {
		OSG_WARN << "Manipulator warning: eventTimeDelta = " << event_time_delta << std::endl;
		event_time_delta = 0.;
	}

	// get deltaX and deltaY
	float dx = _ga_t0->getXnormalized() - _ga_t1->getXnormalized();
	float dy = _ga_t0->getYnormalized() - _ga_t1->getYnormalized();

	// return if there is no movement.
	if (dx == 0. && dy == 0.)
		return false;

	// call appropriate methods
	unsigned int button_mask = _ga_t1->getButtonMask();
	unsigned int mod_key_mask = _ga_t1->getModKeyMask();

	if (_drag_first) {
		if ((!_drag_mod_key || mod_key_mask & _drag_mod_key) && button_mask == _drag_mouse) {
			return PerformMovementDrag(event_time_delta, dx, dy);
		}
		else if ((!_rotate_mod_key || mod_key_mask & _rotate_mod_key) && button_mask == _rotate_mouse) {
			return PerformMovementRotate(event_time_delta, dx, dy);
		}
	}
	else {
		if ((!_rotate_mod_key || mod_key_mask & _rotate_mod_key) && button_mask == _rotate_mouse) {
			return PerformMovementRotate(event_time_delta, dx, dy);
		}
		else if ((!_drag_mod_key || mod_key_mask & _drag_mod_key) && button_mask == _drag_mouse) {
			return PerformMovementDrag(event_time_delta, dx, dy);
		}
	}

	return false;
}
