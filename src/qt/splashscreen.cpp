// Copyright (c) 2011-2021 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#if defined(HAVE_CONFIG_H)
#include <config/bitcoin-config.h>
#endif

#include <qt/splashscreen.h>

#include <clientversion.h>
#include <interfaces/handler.h>
#include <interfaces/node.h>
#include <interfaces/wallet.h>
#include <qt/guiutil.h>
#include <qt/networkstyle.h>
#include <qt/walletmodel.h>
#include <util/system.h>
#include <util/translation.h>

#include <functional>

#include <QApplication>
#include <QCloseEvent>
#include <QPainter>
#include <QRadialGradient>
#include <QScreen>


SplashScreen::SplashScreen(const NetworkStyle* networkStyle)
    : QWidget(nullptr, Qt::FramelessWindowHint), curAlignment(0)
{
    // set reference point, paddings
    int paddingRight            = 30;
    int paddingTop              = 50;
    int titleVersionVSpace      = 17;
    int titleCopyrightVSpace    = 40;
    int titleCoinVSpace         = 25;

    float fontFactor            = 1.0;
    float devicePixelRatio      = 1.0;
    devicePixelRatio = static_cast<QGuiApplication*>(QCoreApplication::instance())->devicePixelRatio();

    // define text to place
    QString titleText       = PACKAGE_NAME;
    QString coinText       = "MateableCoin";

    QString versionText     = QString("Version %1").arg(QString::fromStdString(FormatFullVersion()));
    //QString copyrightText   = QString::fromUtf8(CopyrightHolders(strprintf("\xc2\xA9 %u-%u ", 2009, COPYRIGHT_YEAR)).c_str());
    const QString& titleAddText    = networkStyle->getTitleAddText();

    QString font            = QApplication::font().toString();

    // create a bitmap according to device pixelratio
    QSize splashSize(480*devicePixelRatio,320*devicePixelRatio);
    pixmap = QPixmap(splashSize);

    // change to HiDPI if it makes sense
    pixmap.setDevicePixelRatio(devicePixelRatio);

    QPainter pixPaint(&pixmap);
    pixPaint.fillRect(pixmap.rect(), Qt::black); // Clearing pixmap with black color
    pixPaint.setPen(Qt::white); // Set text color to white

    QPixmap backgroundImage(":/icons/splash");
    backgroundImage = backgroundImage.scaled(pixmap.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    pixPaint.drawPixmap(0, 0, backgroundImage);


// Calculate the desired size for the icon
const int iconSize = 180; // Desired icon size
int iconX = 15; // Horizontal position of the icon (left)
int iconY = 20; // Vertical position of the icon (top)

// Define the rect for the icon based on the desired position and size
QRect rectIcon(QPoint(iconX, iconY), QSize(iconSize, iconSize));

// Load the icon with the desired size
QPixmap icon(networkStyle->getAppIcon().pixmap(iconSize, iconSize));
pixPaint.drawPixmap(rectIcon, icon);

// Calculate the width of the coin text based on the font size and scale factor
QFontMetrics fm(QFont(font, 15 * fontFactor));  // Declare once and reuse
int coinTextWidth = fm.width(coinText);  // Get the width of the "MateableCoin" text

// Calculate the horizontal position to center the coin text under the icon
int coinTextX = iconX + (iconSize - coinTextWidth) / 2; // Center the coin text horizontally with respect to the icon

// Calculate the vertical position for the coin text (below the icon)
int coinTextY = iconY + iconSize + titleCoinVSpace; // Position text below the icon with the desired spacing

// Draw the "Mateable" text
pixPaint.setFont(QFont(font, 15 * fontFactor, QFont::Bold));
pixPaint.setPen(Qt::white); // Set text color to white
pixPaint.drawText(coinTextX, coinTextY, "Mateable");

// Calculate the width of the "Mateable" text to properly position "Coin"
coinTextWidth = fm.width("Mateable");  // Calculate width of the "Mateable" part

// Adjust position for "Coin" text to follow "Mateable"
coinTextX += coinTextWidth + 13; // Add some spacing after "Mateable"

// Draw the "Coin" text
pixPaint.setPen(Qt::white); // Set text color to white
pixPaint.drawText(coinTextX, coinTextY, "Coin");


// Define a rightward offset (in pixels)
int rightOffset = -70;//Adjust this value to move the text further right

// Initialize font and painter
QFont titleFont(font, 33 * fontFactor, QFont::Bold);
pixPaint.setFont(titleFont);
fm = pixPaint.fontMetrics();

// Check and adjust title text width
int titleTextWidth = GUIUtil::TextWidth(fm, titleText);
if (titleTextWidth > 176) {
    fontFactor *= 176.0 / titleTextWidth;
    titleFont.setPointSizeF(33 * fontFactor);
    pixPaint.setFont(titleFont);
    fm = pixPaint.fontMetrics();
    titleTextWidth = GUIUtil::TextWidth(fm, titleText);
}

// Calculate title text position with rightward offset
int titleX = (pixmap.width() / devicePixelRatio - titleTextWidth - paddingRight + rightOffset); // Move to the right
pixPaint.drawText(titleX, paddingTop, titleText);

// Version text
QFont versionFont(font, 15 * fontFactor, QFont::Bold);
pixPaint.setFont(versionFont);
fm = pixPaint.fontMetrics();

int versionTextWidth = GUIUtil::TextWidth(fm, versionText);
if (versionTextWidth > titleTextWidth + paddingRight - 10) {
    versionFont.setPointSizeF(10 * fontFactor);
    pixPaint.setFont(versionFont);
    titleVersionVSpace -= 5;
}

pixPaint.drawText(
    titleX, // Align with title text and right offset
    paddingTop + titleVersionVSpace,
    versionText
);

// Copyright text
QFont copyrightFont(font, 15 * fontFactor, QFont::Bold);
pixPaint.setFont(copyrightFont);
const int copyrightY = paddingTop + titleCopyrightVSpace;

QRect copyrightRect(
    titleX, // Align with title text and right offset
    copyrightY,
    pixmap.width() / devicePixelRatio - titleX - paddingRight,
    pixmap.height() - copyrightY
);

pixPaint.drawText(
    copyrightRect,
    Qt::AlignLeft | Qt::AlignTop,
    getClientCopyright()
);

// Additional network-specific text
if (!titleAddText.isEmpty()) {
    QFont addTextFont(font, 10 * fontFactor, QFont::Bold);
    pixPaint.setFont(addTextFont);
    fm = pixPaint.fontMetrics();

    int titleAddTextWidth = GUIUtil::TextWidth(fm, titleAddText);
    int titleAddTextX = (pixmap.width() / devicePixelRatio - titleAddTextWidth - 10 + rightOffset); // Move to the right
    pixPaint.drawText(
        titleAddTextX,
        paddingTop + 15,
        titleAddText
    );
}


    pixPaint.end();

    // Set window title
    setWindowTitle(titleText + " " + titleAddText);

    // Resize window and move to center of desktop, disallow resizing
    QRect r(QPoint(), QSize(pixmap.size().width()/devicePixelRatio,pixmap.size().height()/devicePixelRatio));
    resize(r.size());
    setFixedSize(r.size());
    move(QGuiApplication::primaryScreen()->geometry().center() - r.center());

    installEventFilter(this);

    GUIUtil::handleCloseWindowShortcut(this);
}

QString SplashScreen::getClientCopyright()
{
    std::string clientName(CLIENT_NAME);

    // Convert clientName to lowercase for case-insensitive comparison
    std::transform(clientName.begin(), clientName.end(), clientName.begin(), ::tolower);

    size_t mateableIndex = clientName.find("mateable");
    size_t bitcoinIndex = clientName.find("bitcoin");

    QString mateableCopyright = QString("\u00A9 %1-%2 The Mateable Core Developers")
        .arg(QString::number(2022), QString::number(COPYRIGHT_YEAR));

    QString bitcoinCopyright = QString("\u00A9 %1-%2 The Bitcoin Core Developers")
        .arg(QString::number(2009), QString::number(COPYRIGHT_YEAR));

    if (mateableIndex != std::string::npos && bitcoinIndex != std::string::npos) {
        // Both Mateable and Bitcoin Core found
        return mateableCopyright + "\n" + bitcoinCopyright;
    }

    // Neither Mateable nor Bitcoin Core found
    return "";
}

SplashScreen::~SplashScreen()
{
    if (m_node) unsubscribeFromCoreSignals();
}

void SplashScreen::setNode(interfaces::Node& node)
{
    assert(!m_node);
    m_node = &node;
    subscribeToCoreSignals();
    if (m_shutdown) m_node->startShutdown();
}

void SplashScreen::shutdown()
{
    m_shutdown = true;
    if (m_node) m_node->startShutdown();
}

bool SplashScreen::eventFilter(QObject * obj, QEvent * ev) {
    if (ev->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(ev);
        if (keyEvent->key() == Qt::Key_Q) {
            shutdown();
        }
    }
    return QObject::eventFilter(obj, ev);
}

void SplashScreen::finish()
{
    /* If the window is minimized, hide() will be ignored. */
    /* Make sure we de-minimize the splashscreen window before hiding */
    if (isMinimized())
        showNormal();
    hide();
    deleteLater(); // No more need for this
}
static void InitMessage(SplashScreen *splash, const std::string &message)
{
    bool invoked = QMetaObject::invokeMethod(splash, "showMessage",
        Qt::QueuedConnection,
        Q_ARG(QString, QString::fromStdString(message)),
        Q_ARG(int, Qt::AlignBottom|Qt::AlignHCenter),
        Q_ARG(QColor, QColor(255, 255, 255))); // Change color to white
    assert(invoked);
}

static void ShowProgress(SplashScreen *splash, const std::string &title, int nProgress, bool resume_possible)
{
    InitMessage(splash, title + std::string("\n") +
            (resume_possible ? SplashScreen::tr("(press q to shutdown and continue later)").toStdString()
                                : SplashScreen::tr("press q to shutdown").toStdString()) +
            strprintf("\n%d", nProgress) + "%");
}

void SplashScreen::subscribeToCoreSignals()
{
    // Connect signals to client
    m_handler_init_message = m_node->handleInitMessage(std::bind(InitMessage, this, std::placeholders::_1));
    m_handler_show_progress = m_node->handleShowProgress(std::bind(ShowProgress, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    m_handler_init_wallet = m_node->handleInitWallet([this]() { handleLoadWallet(); });
}

void SplashScreen::handleLoadWallet()
{
#ifdef ENABLE_WALLET
    if (!WalletModel::isWalletEnabled()) return;
    m_handler_load_wallet = m_node->walletLoader().handleLoadWallet([this](std::unique_ptr<interfaces::Wallet> wallet) {
        m_connected_wallet_handlers.emplace_back(wallet->handleShowProgress(std::bind(ShowProgress, this, std::placeholders::_1, std::placeholders::_2, false)));
        m_connected_wallets.emplace_back(std::move(wallet));
    });
#endif
}

void SplashScreen::unsubscribeFromCoreSignals()
{
    // Disconnect signals from client
    m_handler_init_message->disconnect();
    m_handler_show_progress->disconnect();
    for (const auto& handler : m_connected_wallet_handlers) {
        handler->disconnect();
    }
    m_connected_wallet_handlers.clear();
    m_connected_wallets.clear();
}

void SplashScreen::showMessage(const QString &message, int alignment, const QColor &color)
{
    curMessage = message;
    curAlignment = alignment;
    curColor = color;
    update();
}

void SplashScreen::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.drawPixmap(0, 0, pixmap);
    QRect r = rect().adjusted(5, 5, -5, -5);
    painter.setPen(curColor);

    painter.drawText(r, curAlignment, curMessage);
}

void SplashScreen::closeEvent(QCloseEvent *event)
{
    shutdown(); // allows an "emergency" shutdown during startup
    event->ignore();
}
