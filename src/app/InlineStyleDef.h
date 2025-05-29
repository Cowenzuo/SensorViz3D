#pragma once
#define OverWriteStyle  R"(
        * {
            font-family: "Microsoft Yahei";
            outline: none;
        }
    )"

#define MenuStyle  R"(
        QMenu {
        	border: 1px solid #293447;
        	background: #21252E;
        	color: rgba(255, 255, 255, 1);
        	font-size: 14px;
        	border-radius: 3px;
        	padding: 5px 5px;
        }
        QMenu::icon {
        	padding: 0px 0px 0px 12px;
        }
        QMenu::indicator {
        	width: 0px;
        	height: 0px;
        }
        QMenu::item {
            padding: 6px 25px 6px 8px;
        }
        QMenu::item:enabled:selected {
            background-color: rgba(185, 212, 255, 0.1);
            border-radius: 6px;
        }
        QMenu::item:disabled {
            color: rgba(135, 135, 135, 0.8);
        }
        QMenu::item:checked {
            background-color: rgba(100, 149, 237, 0.1); /* 比选中状态更深的蓝色调 */
            border-radius: 6px;
            color: #6495ED; /* 强调文字颜色 */
            padding-left: 6px; /* 微调内边距保持对齐 */
        }
        QMenu::item:checked:selected {
            background-color: rgba(100, 149, 237, 0.2); /* 组合选中时的叠加效果 */
        }
    )"

#define BaseDialogStyle R"(
        BaseDialog {
            border-image: url(:/image/common/dialog/bg.png) 6px;
        	border-width: 6px;
        	background: transparent;
        }
        BaseDialogHeaderWidget #base_dialog_title {
            font-size: 20px;
            color: #FFFFFF;
            padding-left: 0px;
            padding-top: 0px;
            padding-bottom: 0px;
        }
        BaseDialogHeaderWidget #base_dialog_close {
            border-image: url(:/image/common/dialog/close_normal.png);
            width: 24px;
            height: 24px;
        }
        BaseDialogHeaderWidget #base_dialog_close:hover{
            border-image: url(:/image/common/dialog/close_hover.png);
        }
        BaseDialogHeaderWidget #base_dialog_close:pressed {
            border-image: url(:/image/common/dialog/close_pressed.png);
        }
 )"

#define MsgBoxStyle R"(
        OpeMessageBox #dialog_bg {
        	border-image: url(:/image/common/messagebox/background.png) 6px;
        	border-width: 6px;
        	min-width: 380px;
        	max-width: 380px;
        }
        OpeMessageBox #dialog_header_widget {
            
        }
        OpeMessageBox #dialog_title_label {
            font-size: 20px;
            font-weight: bold;
            color: rgba(255, 255, 255, 1);
            padding: 0px;
        }
        OpeMessageBox #dialog_close_button {
            border-image: url(:/image/common/dialog/close_normal.png);
            width: 24px;
            height: 24px;
        }
        OpeMessageBox #dialog_close_button:hover {
            border-image: url(:/image/common/dialog/close_hover.png);
        }
        OpeMessageBox #dialog_close_button:pressed {
            border-image: url(:/image/common/dialog/close_pressed.png);
        }
        OpeMessageBox #msgbox_content {
            font-size: 14px;
            color: #FFFFFF;
            padding: 26px 30px 30px;
        }
        OpeMessageBox QDialogButtonBox QPushButton {
        	outline: none;
        	color: #FFFFFF;
        	font-size: 14px;
            border-image: url(:/image/common/button/secondary.png) 4px;
        	border-width: 4px;
        	padding: 5px 12px;
        }
        OpeMessageBox QDialogButtonBox QPushButton:hover {
            border-image: url(:/image/common/button/secondary_hover.png) 4px;
        }
        OpeMessageBox QDialogButtonBox QPushButton:pressed {
            border-image: url(:/image/common/button/secondary_hover.png) 4px;
        }
        OpeMessageBox QDialogButtonBox QPushButton:focus {
            border-image: url(:/image/common/button/primary.png) 4px;
        }
        OpeMessageBox QDialogButtonBox QPushButton:focus:hover {
            border-image: url(:/image/common/button/primary_hover.png) 4px;
        }
        OpeMessageBox QDialogButtonBox QPushButton:focus:pressed {
            border-image: url(:/image/common/button/primary_hover.png) 4px;
        }
 )"

#define ComboBoxStyle R"(
        QComboBox {
        	border-image: url(:/image/common/combobox/bg.png) 4px;
        	border-width: 4px;
            font-size: 14px;
        	color: #FFFFFF;
        	height: 24px;
            padding: 0 4px;
        }
        QComboBox:hover {
        	border-image: url(:/image/common/combobox/bg_hover.png) 4px;
        }
        QComboBox:focus {
        	border-image: url(:/image/common/combobox/bg_focus.png) 4px;
        }
        QComboBox:disabled {
        	border-image: url(:/image/common/combobox/bg_disabled.png) 4px;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: center right;
            width: 30px;
            height: 30px;
        	image: url(:/image/common/combobox/drop_down.png);
        }
        QComboBox::drop-down:hover {
        	image: url(:/image/common/combobox/drop_down_hover.png);
        }
        QComboBox::drop-down:disabled {
        	image: url(:/image/common/combobox/drop_down_disabled.png);
        }
        QComboBox::drop-down:on {
        	image: url(:/image/common/combobox/drop_down_on.png);
        }
        QComboBox::drop-down:on:hover {
        	image: url(:/image/common/combobox/drop_down_on_hover.png);
        }
        QComboBox QListView {
        	border: 1px solid #293447;
        	background: #21252E;
        	color: rgba(255, 255, 255, 1);
        	font-size: 14px;
        	padding: 5px 5px;
        }
 )"

#define CustomFlowWidgetItemStyle R"(
        background:none;
        border:1px solid white;
)"

#define QScrollAreaStyle R"(
        QScrollArea {
            background: transparent;
            border-image: url(:/image/background.png);
        }
        QScrollArea>QWidget>QWidget {
            background: transparent;
        }
        QScrollBar {
        	background: transparent;
        	border: none;
        }
        QScrollBar:vertical {
        	width: 8px;
        }
        QScrollBar:horizontal {
        	height: 8px;
        }
        QScrollBar::handle:vertical {
        	min-height: 60px;
        	width: 8px;
        	background: qlineargradient(x1:0.5, y1:0, x2:0.5, y2:1, stop:0 rgba(99,126,173,0.3), stop:1 rgba(33,44,61,0.3));
        	border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
        	background:qlineargradient(x1:0.5, y1:0, x2:0.5, y2:1,stop:0 #637EAD,stop:1 #39485E);
        }
        QScrollBar::handle:horizontal {
        	min-width: 60px;
        	height: 8px;
        	background: qlineargradient(x1:0, y1:0.5, x2:1, y2:0.5, stop:0 rgba(99,126,173,0.3), stop:1 rgba(33,44,61,0.3));
        	border-radius: 4px;
        }
        QScrollBar::handle:horizontal:hover {
        	background: qlineargradient(x1:0, y1:0.5, x2:1, y2:0.5, stop:0 #637EAD, stop:1 #39485E);
        }
        QScrollBar::add-line {
        	subcontrol-position: bottom;
            subcontrol-origin: padding;
        	width: 0px;
            height: 0px;
            border: none;
        }
        QScrollBar::sub-line {
        	subcontrol-position: top;
            subcontrol-origin: padding;
        	width: 0px;
            height: 0px;
            border: none;
        }
        QScrollBar::add-page, QScrollBar::sub-page {
        	background:transparent;
        }
)"

#define QCheckBoxStyle R"(
        QCheckBox::indicator, QListView::indicator, QTreeView::indicator {
            width: 16px;
            height: 16px;
        	border: none;
        }
        QCheckBox::indicator, QListView::indicator, QTreeView::indicator {
        	image: url(:/image/common/checkbox/unchecked.png);
        }
        QCheckBox::indicator:hover, QListView::indicator:hover, QTreeView::indicator:hover {
        	image: url(:/image/common/checkbox/unchecked_hover.png);
        }
        QCheckBox::indicator:pressed, QListView::indicator:pressed, QTreeView::indicator:pressed {
        	image: url(:/image/common/checkbox/unchecked_hover.png);
        }
        QCheckBox::indicator:disabled, QListView::indicator:disabled, QTreeView::indicator:disabled {
        	image: url(:/image/common/checkbox/unchecked_disabled.png);
        }
        QCheckBox::indicator:indeterminate, QListView::indicator:indeterminate, QTreeView::indicator:indeterminate {
        	image: url(:/image/common/checkbox/partchecked.png);
        }
        QCheckBox::indicator:indeterminate:hover, QListView::indicator:indeterminate:hover, QTreeView::indicator:indeterminate:hover {
        	image: url(:/image/common/checkbox/partchecked_hover.png);
        }
        QCheckBox::indicator:indeterminate:pressed, QListView::indicator:indeterminate:pressed, QTreeView::indicator:indeterminate:pressed {
        	image: url(:/image/common/checkbox/partchecked_hover.png);
        }
        QCheckBox::indicator:indeterminate:disabled, QListView::indicator:indeterminate:disabled, QTreeView::indicator:indeterminate:disabled {
        	image: url(:/image/common/checkbox/partchecked_disabled.png);
        	color: rgba(255, 255, 255, 0.3);
        }
        QCheckBox::indicator:checked, QListView::indicator:checked, QTreeView::indicator:checked {
        	image: url(:/image/common/checkbox/checked.png);
        }
        QCheckBox::indicator:checked:hover, QListView::indicator:checked:hover,QTreeView::indicator:checked:hover {
        	image: url(:/image/common/checkbox/checked_hover.png);
        }
        QCheckBox::indicator:checked:pressed, QListView::indicator:checked:pressed,QTreeView::indicator:checked:pressed {
        	image: url(:/image/common/checkbox/checked_hover.png);
        }
        QCheckBox::indicator:checked:disabled, QListView::indicator:checked:disabled, QTreeView::indicator:checked:disabled {
        	image: url(:/image/common/checkbox/checked_disabled.png);
        }
)"

#define QLabelStyle R"(
        QLabel{
        	color: #cdf3ff;
        	font-size:18px;
        }

)"

#define QSliderStyle R"(
        QSlider::groove:horizontal{
        	background: #ecf0f1;
        	height: 10px;
        	border-radius: 5px;
        }
        
        QSlider::sub-page:horizontal{
        	background: #3498db;
        	height: 10px;
        	border - radius: 5px;
        }
        
        QSlider::handle:horizontal{
        	background: #3498db;
        	width: 10px;
        	height: 10px;
        	border-radius: 5px;
        	margin:-2px 0;
        }
)"