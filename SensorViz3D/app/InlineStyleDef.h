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