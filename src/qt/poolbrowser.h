#ifndef POOLBROWSER_H
#define POOLBROWSER_H

#include "clientmodel.h"
#include "main.h"
#include "wallet.h"
#include "base58.h"

#include <QWidget>
#include <QObject>
#include <QLabel>
#include <QtNetwork/QtNetwork>
#include <qcustomplot.h>
#include <QCoreApplication>
#include <QDebug>
#include <QApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include <QVariant>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonObject>
#include <QVariantMap>
#include <QJsonArray>

extern QString bitcoing;
extern QString dollarg;

namespace Ui {
class PoolBrowser;
}
class ClientModel;


class PoolBrowser : public QWidget
{
    Q_OBJECT

public:
    explicit PoolBrowser(QWidget *parent = 0);
    ~PoolBrowser();
    
    void setModel(ClientModel *model);

    void refreshValueLabel(double actualValue, QLabel *label, double newDouble, const QString &suffix, QString newStr);
    void refreshUSDLabel(QLabel *label, double actualValue, double newDouble);
private:
    void setupBittrexGraphs();
    void setupBittrexTabSlots();

//    void setupMintPalGraphs();
//    void setupMintPalTabSlots();

    void setupCryptsyGraphs();
    void setupCryptsyTabSlots();

    void getRequest( const QString &url );

    void parseBittrexSummary(QNetworkReply *replay);
    void parseCurrencyUSD(QNetworkReply *replay);
    void parseBittrexOrders(QNetworkReply *replay);
    void parseBittrexHistory(QNetworkReply *replay);

//    void parseMintPalSummary(QNetworkReply *replay);
//    void parseMintPalOrders(QNetworkReply *replay);
//    void parseMintPalHistory(QNetworkReply *replay);

    void parseCryptsySummary(QNetworkReply *replay);
    void parseCryptsyOrders(QNetworkReply *replay);

    void openUrl(const QString &url);

    void downloadAllMarketsData();

    void setColoredTextBasedOnValues(const QString &text, QLabel *label, double prevValue, double actualValue);
    void setRedTextForLabel(const QString &text, QLabel *label);
    void setGreenTextForLabel(const QString &text, QLabel *label);

    double refreshStringVarUsingField(const QJsonObject &jsonObject, const QString &fieldName, double actualValue, QLabel *label, const QString &suffix = "");
    double refreshDoubleVarUsingField(const QJsonObject &jsonObject, const QString &fieldName, double actualValue, QLabel *label, const QString &suffix = "");
    double refreshStringVarAsBTCUsingField(const QJsonObject &jsonObject, const QString &fieldName, double actualValue, QLabel *label);
    double refreshStringVarAsUSDUsingField(const QJsonObject &jsonObject, const QString &fieldName, double actualValue, QLabel *label);
    double refreshDoubleVarAsBTCUsingField(const QJsonObject &jsonObject, const QString &fieldName, double actualValue, QLabel *label);
    double refreshDoubleVarAsUSDUsingField(const QJsonObject &jsonObject, const QString &fieldName, double actualValue, QLabel *label);

    void updateMintPalPricesPlot(const QVector<double> &count, const QVector<double> &prices);
    void updateCrypstyPricesPlot(const QVector<double> &count, const QVector<double> &prices);
    void updatePricesPlot(const QVector<double> &count, const QVector<double> &prices, QCustomPlot *plot);
    void updateVolumeSatoshiPlot(const QVector<double> &satoshiSell, QVector<double> &satoshiBuy, const QVector<double> &volumeSell, const QVector<double> &volumeBuy, QCustomPlot *plot);

    double getMaxValueFromVector(const QVector<double> &vector);
    double getMinValueFromVector(const QVector<double> &vector);

signals:
    void networkError( QNetworkReply::NetworkError err );

public slots:
    void parseNetworkResponse(QNetworkReply *finished);
    void updateBitcoinPrice();
    void downloadBittrexMarketData();
    void downloadMintPalMarketData();
    void downloadCryptsyMarketData();

    void openBittrexPage();
//    void openMintPalPage();
    void openCryptsyPage();
    void egaldo();

private:
    QNetworkAccessManager m_nam;
    Ui::PoolBrowser *ui;
    ClientModel *model;

};

#endif // POOLBROWSER_H
