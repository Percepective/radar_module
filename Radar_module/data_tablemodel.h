#pragma once

#include <QTableView>
#include <QStringList>
#include <QPainter>
#include <QHeaderView>
#include <QAbstractTableModel>
#include "can_thread.h"
#include "IDBCManager.h"

//QAbstractTableModelΪģ���ṩ��һ����׼�ӿڣ���Щģ�ͽ������ݱ�ʾΪһ����ά���顣���������໯��
//�����໯QAbstractTableModelʱ������ʵ��rowCount()��columnCount()��data()��
//index()��parent()������Ĭ��ʵ����QAbstractTableModel�ṩ����Ϊ���õ�ģ�ͻ���Ҫʵ��headerData()��
class Data_TableModel : public QAbstractTableModel
{
	Q_OBJECT

public:

	Data_TableModel(QObject*);
	~Data_TableModel(void);
	void setHorizontalHeaderList(QStringList horizontalHeaderList);
	void setVerticalHeaderList(QStringList verticalHeaderList);
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
	Qt::ItemFlags flags(const QModelIndex &index) const;
	void refresh_with_new_frame();
	//void setModalData(QList< QStringList > *rowlist);
	void refrushModel();

signals:

	void updateCount(int count);

private:
	QStringList horizontal_header_list;
	QStringList vertical_header_list;
	QList< QList<QString>> arr_row_list;
};
