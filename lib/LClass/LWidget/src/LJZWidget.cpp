#include "LJZWidget.h"

#include "func.h"
#ifdef WIN32
#include <Windows.h>
#pragma comment(lib, "user32.lib")
#endif // WIN32
#include <windowsx.h>

using namespace ljz;

LTitleBar::LTitleBar(QWidget *parent)
	: LBaseTitleBar(parent)
{
	this->layout = new QHBoxLayout(this);
	this->layout->setContentsMargins(0, 0, 0, 0);

	/*自动缩放的占位符*/
	QSpacerItem *spacerItem = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	// 再添加一个占位符，使得搜索框居中
	QSpacerItem *spacerItem2 = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

	// 初始化按钮图标
	this->standardIconMap.insert(ButtonIcon::MinButtonIcon, style()->standardIcon(QStyle::SP_TitleBarMinButton));
	this->standardIconMap.insert(ButtonIcon::MaxButtonIcon, style()->standardIcon(QStyle::SP_TitleBarMaxButton));
	this->standardIconMap.insert(ButtonIcon::RestoreButtonIcon, style()->standardIcon(QStyle::SP_TitleBarNormalButton));
	this->standardIconMap.insert(ButtonIcon::CloseButtonIcon, style()->standardIcon(QStyle::SP_TitleBarCloseButton));

	this->minButton = new QPushButton(this);
	this->minButton->setFixedSize(25, 25);
	this->minButton->setIcon(this->standardIconMap.value(ButtonIcon::MinButtonIcon));
	this->minButton->setCursor(Qt::PointingHandCursor);								   // 设置鼠标为手型
	connect(this->minButton, &QPushButton::clicked, this, &LTitleBar::minButtonClick); // 最小化按钮信号
	this->minButton->setToolTip("最小化");

	this->maxButton = new QPushButton(this);
	this->maxButton->setFixedSize(25, 25);
	this->maxButton->setIcon(this->standardIconMap.value(ButtonIcon::MaxButtonIcon));
	this->maxButton->setCursor(Qt::PointingHandCursor);
	this->maxButton->setToolTip("最大化/还原");
	this->maxButton->setObjectName("maxButton");

	this->closeButton = new QPushButton(this);
	this->closeButton->setFixedSize(25, 25);
	this->closeButton->setIcon(this->standardIconMap.value(ButtonIcon::CloseButtonIcon));
	this->closeButton->setCursor(Qt::PointingHandCursor);
	connect(this->closeButton, &QPushButton::clicked, this, &LTitleBar::closeButtonClick);
	this->closeButton->setToolTip("关闭");

	this->layout->addSpacerItem(spacerItem);
	this->layout->addSpacerItem(spacerItem2);
	this->layout->addWidget(this->minButton);
	this->layout->addWidget(this->maxButton);
	this->layout->addWidget(this->closeButton);

	this->setLayout(this->layout);
	this->setTitleText("Title");
	this->icon = style()->standardIcon(QStyle::SP_TitleBarMenuButton);
	this->flag = false;
}

void LTitleBar::setTitleIcon(const QIcon &icon)
{
	this->icon = icon;
}

void LTitleBar::setTitleIcon(const QPixmap &pixmap)
{
	this->icon = QIcon(pixmap);
}

void LTitleBar::setTitleText(const QString &text)
{
	this->title = text;
}

void LTitleBar::updateIcon()
{
	if (this->flag)
	{
		this->maxButton->setIcon(this->standardIconMap.value(ButtonIcon::MaxButtonIcon));
		this->flag = false;
	}
	else
	{
		this->maxButton->setIcon(this->standardIconMap.value(ButtonIcon::RestoreButtonIcon));
		// 还原按钮点击

		this->flag = true;
	}
	update();
}

void LTitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		this->maxButton->click();
		return QWidget::mouseDoubleClickEvent(event);
	}
}

void LTitleBar::focusOutEvent(QFocusEvent *event)
{
	LBaseTitleBar::focusOutEvent(event);
}

QMap<LTitleBar::ButtonIcon, QIcon> LTitleBar::getStandardIconMap() const
{
	return this->standardIconMap;
}

void ljz::LTitleBar::setStandardIconMap(const QMap<ButtonIcon, QIcon> &iconMap)
{
	this->setMinButtonIcon(iconMap.value(ButtonIcon::MinButtonIcon));
	this->setMaxButtonIcon(iconMap.value(ButtonIcon::MaxButtonIcon));
	this->setRestoreButtonIcon(iconMap.value(ButtonIcon::RestoreButtonIcon));
	this->setCloseButtonIcon(iconMap.value(ButtonIcon::CloseButtonIcon));
}

QIcon LTitleBar::setMinButtonIcon(const QIcon &icon)
{
	// 更新最小化按钮图标
	this->standardIconMap.insert(ButtonIcon::MinButtonIcon, icon);
	this->minButton->setIcon(icon);
	return icon;
}

QIcon LTitleBar::setMinButtonIcon(const QString &iconPath)
{
	const QIcon icon(iconPath);
	return this->setMinButtonIcon(icon);
}

QIcon LTitleBar::setMaxButtonIcon(const QIcon &icon)
{
	// 更新最大化按钮图标
	this->standardIconMap.insert(ButtonIcon::MaxButtonIcon, icon);
	this->maxButton->setIcon(icon);
	return icon;
}

QIcon LTitleBar::setMaxButtonIcon(const QString &iconPath)
{
	const QIcon icon(iconPath);
	return this->setMaxButtonIcon(icon);
}

QIcon LTitleBar::setRestoreButtonIcon(const QIcon &icon)
{
	// 更新还原按钮图标
	this->standardIconMap.insert(ButtonIcon::RestoreButtonIcon, icon);
	if (this->parentWidget()->windowState() == Qt::WindowMaximized)
		this->maxButton->setIcon(icon);
	return icon;
}

QIcon LTitleBar::setRestoreButtonIcon(const QString &iconPath)
{
	const QIcon icon(iconPath);
	return this->setRestoreButtonIcon(icon);
}

QIcon LTitleBar::setCloseButtonIcon(const QIcon &icon)
{
	// 更新关闭按钮图标
	this->standardIconMap.insert(ButtonIcon::CloseButtonIcon, icon);
	this->closeButton->setIcon(icon);
	return icon;
}

QIcon LTitleBar::setCloseButtonIcon(const QString &iconPath)
{
	const QIcon icon(iconPath);
	return this->setCloseButtonIcon(icon);
}

void LTitleBar::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing); // 反锯齿
	// 绘制icon,大小为30，30
	painter.drawPixmap(2, 2, 17, 17, this->icon.pixmap(17, 17));
	// 字体大小
	QFont font;
	font.setPointSize(12);
	// 微软雅黑
	font.setFamily("Microsoft YaHei");
	painter.setFont(font);

	// 绘制标题
	painter.drawText(40, 0, this->width() - 40, 25, Qt::AlignLeft | Qt::AlignVCenter, this->title);
	painter.end();
	update();
}

void LTitleBar::closeButtonClick()
{
	emit LTitleBar::closeButtonClicked();
}

void LTitleBar::minButtonClick()
{
	emit LTitleBar::minButtonClicked();
}

void LTitleBar::addWidget(QWidget *widget)
{
	// 在最小化左边添加widget也就是索引为0的widget
	widget->setFixedHeight(25);
	this->layout->insertWidget(2, widget);
}
LWidget::LWidget(LBaseTitleBar *titleBar, QWidget *mainWidget, QWidget *parent)
	: QWidget(parent)
{
	this->setWindowFlag(Qt::FramelessWindowHint);													   // 无边框
	this->setAttribute(Qt::WA_TranslucentBackground);												   // 背景透明
	this->_layout->setContentsMargins(_info.edgeSize, _info.edgeSize, _info.edgeSize, _info.edgeSize); // 设置布局边距
	this->_layout->setSpacing(0);																	   // 设置布局间距
	this->setLayout(this->_layout);																	   // 设置布局
#ifdef Q_OS_WIN
	HWND hwnd = reinterpret_cast<HWND>(this->winId());
	DWORD style = ::GetWindowLong(hwnd, GWL_STYLE);
	::SetWindowLong(hwnd, GWL_STYLE, style | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CAPTION | WS_SYSMENU);
#endif
	// 安装事件过滤器识别拖动
	this->installEventFilter(this);
	this->setTitleBar(titleBar);
	this->setMainWidget(mainWidget);
	setFocusPolicy(Qt::FocusPolicy::StrongFocus);
}

void LWidget::loadLayout() const
{
	while (this->_layout->count())
	{
		this->_layout->takeAt(0); // 移除布局
	}
	if (this->_titleBar != nullptr)
	{
		this->_layout->addWidget(this->_titleBar); // 添加标题栏
	}
	if (this->_menuBar != nullptr)
	{
		this->_layout->addWidget(this->_menuBar); // 添加菜单栏
	}
	if (this->_mainWidget != nullptr)
	{
		this->_layout->addWidget(this->_mainWidget); // 添加主窗口
	}
	if (this->_statusBar != nullptr)
	{
		this->_layout->addWidget(this->_statusBar); // 添加状态栏
	}
}

void LWidget::showCustomNormal()
{
	if (this->isMaximized() == false)
	{
		this->showMaximized(); // 最大化
	}
	else
	{
		this->showNormal();
	}
}

void LWidget::setTitleBar(LBaseTitleBar *titleBar)
{
	this->_titleBar = titleBar;
	this->_titleBar->setMouseTracking(true);   // 设置鼠标跟踪
	this->_titleBar->setFixedHeight(25);	   // 设置标题栏高度
	this->_titleBar->installEventFilter(this); // 安装事件过滤器识别拖动
	connect(this->_titleBar, &LBaseTitleBar::closeButtonClicked, this, &LWidget::close);
	connect(this->_titleBar, &LBaseTitleBar::minButtonClicked, this, &LWidget::showMinimized);
	connect(this, &LWidget::windowStateChanged, this->_titleBar, &LBaseTitleBar::updateIcon);
	this->loadLayout();
}

void LWidget::setMenuBar(QWidget *menuBar)
{
	this->_menuBar = menuBar;
	this->_menuBar->setFixedHeight(30);
	this->loadLayout();
}

void LWidget::setMainWidget(QWidget *mainWidget)
{
	if (mainWidget == nullptr)
		return;
	this->_mainWidget = mainWidget;
	this->_mainWidget->installEventFilter(this);
	this->loadLayout();
}

void LWidget::setStatusBar(QWidget *statusBar)
{
	this->_statusBar = statusBar;
	this->_statusBar->setFixedHeight(20);
	this->loadLayout();
}

void LWidget::setInfo(const Info &info)
{
	this->_info = info;
	this->_layout->setContentsMargins(_info.edgeSize, _info.edgeSize, _info.edgeSize, _info.edgeSize);
}

LWidget::Info LWidget::info() const
{
	return this->_info;
}

void LWidget::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing); // 反锯齿
	painter.setPen(Qt::NoPen);					   // 无边框
#ifdef Q_OS_WIN
	painter.setBrush(LFunc::getSystemAccentColor()); // 边框颜色
#endif
	painter.drawRoundedRect(this->rect(), _info.radius, _info.radius); // 绘制边框

	// 再绘制一层背景颜色的圆角矩形，大小为窗口大小减去边框大小
	// 获取当前的系统主题色
	QColor color = this->palette().color(QPalette::Window);
	painter.setBrush(color); // 背景颜色
	painter.drawRoundedRect(this->rect().adjusted(_info.borderSize, _info.borderSize,
												  -_info.borderSize, -_info.borderSize),
							_info.radius, _info.radius); // 绘制背景
	if (_info.backgroundPixmap.isNull() == false)
	{
		QPixmap pixmap = _info.backgroundPixmap.scaled(this->rect().adjusted(
																	   _info.borderSize, _info.borderSize, -_info.borderSize, -_info.borderSize)
														   .size());
		painter.setBrush(QBrush(pixmap));
		painter.drawRoundedRect(this->rect().adjusted(
									_info.borderSize, _info.borderSize, -_info.borderSize, -_info.borderSize),
								_info.radius, _info.radius);
	}
	for (int i = 1; i < this->_layout->count(); i++)
	{
		int x = this->_layout->itemAt(i)->geometry().x();
		int y = this->_layout->itemAt(i)->geometry().y() - 1;
		int w = this->_layout->itemAt(i)->geometry().width();
		int h = 1;
		// 绘制分割线
		painter.setPen(QPen(_info.splitLineColor, 1));
		painter.drawLine(x, y, x + w, y + h);
	}
	painter.setBrush(LFunc::getSystemAccentColor());
	QRect rect(0, 0, this->width(), this->_titleBar->geometry().bottom());
	painter.drawRoundedRect(rect, 0, 0);
	painter.end();
}
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)) // Qt6
bool LWidget::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
#else
bool LWidget::nativeEvent(const QByteArray &eventType, void *message, long *result)
#endif
{
#ifdef WIN32
	if (eventType == "windows_generic_MSG")
	{
		MSG *msg = static_cast<MSG *>(message); // 转换类型
		// 不同的消息类型和参数进行不同的处理
		if (msg->message == WM_NCCALCSIZE)
		{ // 如果是计算窗口大小消息
			*result = 0;
			return true;
		}
		else if (msg->message == WM_SYSKEYDOWN)
		{
			// 如果是alt键按下
			// 屏蔽alt键按下
		}
		else if (msg->message == WM_SYSKEYUP)
		{
			// 如果是alt键松开
			// 屏蔽alt键松开
		}
		else if (msg->message == WM_NCHITTEST)
		{										// 如果是鼠标消息
			long x = GET_X_LPARAM(msg->lParam); // 获取鼠标x坐标
			long y = GET_Y_LPARAM(msg->lParam); // 获取鼠标y坐标
			QPoint pos = mapFromGlobal(QPoint(x, y));
			int padding = _info.edgeSize; // 鼠标距离窗口边缘的距离
			// 判断当前鼠标位置在哪个区域
			bool left = pos.x() < padding;
			bool right = pos.x() > width() - padding;
			bool top = pos.y() < padding;
			bool bottom = pos.y() > height() - padding;
			bool resizeEnable = true; // 是否允许改变窗口大小
			// 鼠标移动到四个角,这个消息是当鼠标移动或者有鼠标键按下时候发出的
			*result = 0;
			if (resizeEnable)
			{
				if (left && top)
				{
					*result = HTTOPLEFT; //
				}
				else if (left && bottom)
				{
					*result = HTBOTTOMLEFT;
				}
				else if (right && top)
				{
					*result = HTTOPRIGHT;
				}
				else if (right && bottom)
				{
					*result = HTBOTTOMRIGHT;
				}
				else if (left)
				{
					*result = HTLEFT;
				}
				else if (right)
				{
					*result = HTRIGHT;
				}
				else if (top)
				{
					*result = HTTOP;
				}
				else if (bottom)
				{
					*result = HTBOTTOM;
				}
			}

			// 先处理掉拉伸
			if (0 != *result)
			{
				this->update();
				return true;
			}
			// 识别标题栏拖动产生半屏全屏效果
			if (_titleBar && _titleBar->geometry().contains(pos))
			{
				// 如果鼠标在标题栏上,不知道什么原因，需要对pos进行边框宽度的处理，处理后正常，不处理就有点问题
				QWidget *child = _titleBar->childAt(pos - QPoint(2 * _info.borderSize, 2 * _info.borderSize));

				if (!child)
				{
					// 如果标题栏上没有控件
					*result = HTCAPTION; // 设置为标题栏
					return true;
				}
				else if (child == this->_titleBar->getMaxButton())
				{
					*result = HTMAXBUTTON;
					return true;
				}
			}
		}
		else if (msg->wParam == PBT_APMSUSPEND && msg->message == WM_POWERBROADCAST)
		{
			// 系统休眠的时候自动最小化可以规避程序可能出现的问题
			this->hide();
		}
		// else if (msg->wParam == PBT_APMRESUMEAUTOMATIC)
		//{
		//	//休眠唤醒后自动打开
		//	this->showCustomNormal();
		// }
		else if (msg->message == WM_NCRBUTTONUP) // 如果是鼠标右键
		{
			// 获取系统菜单的句柄
			HMENU hMenu = GetSystemMenu(reinterpret_cast<HWND>(this->winId()), FALSE);

			// 获取鼠标位置
			POINT cursor;
			GetCursorPos(&cursor);

			// 设置前台窗口
			SetForegroundWindow(reinterpret_cast<HWND>(this->winId()));

			// 显示系统菜单
			switch (TrackPopupMenu(hMenu, TPM_LEFTBUTTON | TPM_RETURNCMD, cursor.x, cursor.y, 0, reinterpret_cast<HWND>(this->winId()), NULL))
			{
			case SC_RESTORE: // 还原
				ShowWindow(reinterpret_cast<HWND>(this->winId()), SW_RESTORE);
				break;
			case SC_MOVE: // 移动
				SendMessage(reinterpret_cast<HWND>(this->winId()), WM_SYSCOMMAND, SC_MOVE, 0);
				break;
			case SC_SIZE: // 大小
				SendMessage(reinterpret_cast<HWND>(this->winId()), WM_SYSCOMMAND, SC_SIZE, 0);
				break;
			case SC_MINIMIZE: // 最小化
				ShowWindow(reinterpret_cast<HWND>(this->winId()), SW_MINIMIZE);
				break;
			case SC_MAXIMIZE: // 最大化
				ShowWindow(reinterpret_cast<HWND>(this->winId()), SW_MAXIMIZE);
				break;
			case SC_CLOSE: // 关闭
				SendMessage(reinterpret_cast<HWND>(this->winId()), WM_CLOSE, 0, 0);
				break;
			default:
				break;
			}
			*result = 0;
			// 发送消息给窗口以告知菜单已关闭
			PostMessage(reinterpret_cast<HWND>(this->winId()), WM_NULL, 0, 0);
			return true;
		}
		else if (msg->message == WM_SETTINGCHANGE)
		{
			emit this->systemSettingsChanged();
		}
	}
#endif
	return QWidget::nativeEvent(eventType, message, result);
}

bool LWidget::eventFilter(QObject *obj, QEvent *event)
{
	// qDebug() << event->type();
	if (event->type() == QEvent::WindowStateChange)
	{
		emit windowStateChanged();
	}

	return QWidget::eventFilter(obj, event);
}

void LWidget::focusOutEvent(QFocusEvent *event)
{
	QWidget::focusOutEvent(event);
}

LLabelWidgetFrame::LLabelWidgetFrame(QString labelName, QWidget *valueWidget, QWidget *parent)
	: QFrame(parent), _valueWidget(valueWidget)
{
	this->_label->setText(labelName);
	this->_layout->setContentsMargins(0, 0, 0, 0);
	this->_layout->addWidget(this->_label);
	QSpacerItem *spacer = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	this->_layout->addItem(spacer);
	this->_layout->addWidget(this->_valueWidget);
	this->setLayout(this->_layout);
}

void LLabelWidgetFrame::setLabelName(QString labelName)
{
	this->_label->setText(labelName);
}

void LLabelWidgetFrame::setValueWidget(QWidget *valueWidget)
{
	this->_layout->takeAt(2);
	delete this->_valueWidget;
	this->_valueWidget = valueWidget;
	this->_layout->addWidget(this->_valueWidget);
}
