#include "data_tablemodel.h"
#include "global_buffer.h"

Data_TableModel::Data_TableModel(DBCHandle dh, QObject *parent)
	: m_hDBC(dh), QAbstractTableModel(parent), arr_row_list(NULL)
{
	setHorizontalHeaderList(QStringList() << QStringLiteral("ID") << QStringLiteral("��Ϣ��") << QStringLiteral("ע��"));
}

Data_TableModel::~Data_TableModel(void)
{
	arr_row_list = NULL;
}

void Data_TableModel::setHorizontalHeaderList(QStringList horizontalHeaderList)
{
	horizontal_header_list = horizontalHeaderList;
}

void Data_TableModel::setVerticalHeaderList(QStringList verticalHeaderList)
{
	vertical_header_list = verticalHeaderList;
}

//Returns the number of rows under the given parent. 
//When the parent is valid it means that rowCount() is returning the number of children of parent.
//When implementing a table based model, rowCount() should return 0 when the parent is valid.
int Data_TableModel::rowCount(const QModelIndex &parent) const
{
	if (vertical_header_list.size() > 0)
		return vertical_header_list.size();

	if (NULL == arr_row_list)
		return 0;
	else
		return arr_row_list->size();
}

//Returns the number of columns for the children of the given parent.
int Data_TableModel::columnCount(const QModelIndex &parent) const
{
	if (horizontal_header_list.size() > 0)
		return horizontal_header_list.size();

	if (NULL == arr_row_list)
		return 0;
	else if (arr_row_list->size() < 1)
		return 0;
	else
		return arr_row_list->at(0).size();
}

//Returns the data stored under the given role for the item referred to by the index.
QVariant Data_TableModel::data(const QModelIndex &index, int role) const
{
	//Qmodelindex������QAbstractItemModel������item������
	//��Ŀ��ͼ��ί�к�ѡ��ģ����Ҫʹ����������λģ���е���Ŀ��
	if (!index.isValid()) {
		return QVariant();
	}

	if (NULL == arr_row_list) {
		return QVariant();
	}

	if (arr_row_list->size() < 1) {
		return QVariant();
	}

	//ʹ��Ĭ��ί��չʾ��item���ı����뷽ʽ
	if (role == Qt::TextAlignmentRole) {
		return int(Qt::AlignLeft | Qt::AlignVCenter);
	}

	//���ı���ʽ��ʾ�ĺ�������
	else if (role == Qt::DisplayRole) {
		if (index.row() >= arr_row_list->size())
			return QVariant();
		if (index.column() >= arr_row_list->at(0).size())
			return QVariant();
		return arr_row_list->at(index.row()).at(index.column());
	}
	return QVariant();
}

//Returns the data for the given role and section in the header with the specified orientation(�����������).
QVariant Data_TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal) // ˮƽ��ͷ
		{
			if (horizontal_header_list.size() > section)
				return horizontal_header_list[section];
			else
				return QVariant();
		}
		else {
			if (vertical_header_list.size() > section)
				return vertical_header_list[section]; // ��ֱ��ͷ
			else
				return QVariant();
		}
	}

	return QVariant();
}

//���������ʹ�ã��Ի�ȡ����ģ���ṩ��ÿ��item����Ϣ��
Qt::ItemFlags Data_TableModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	Qt::ItemFlags flag = QAbstractItemModel::flags(index);

	// flag|=Qt::ItemIsEditable // ���õ�Ԫ��ɱ༭,�˴�ע��,��Ԫ���޷����༭  
	return flag;
}
void Data_TableModel::refresh_with_new_frame()
{
	mtx.lock();
	qDebug() << "table mtx locked";
	DBCMessage msg;
	auto cur_info = global_buffer.takeFirst();
	qDebug() << "data length: " << cur_info.vco[0].DataLen;
	for (int i = 0; i < cur_info.noframe; ++i) {
		if (DBC_Analyse(m_hDBC, &cur_info.vco[i], &msg)) {
			qDebug() << "analyse success";
			QStringList _list;
			_list << QString::number(msg.nID) << tr(msg.strName) << tr(msg.strComment);
			arr_row_list->append(_list);
		}
		refrushModel();
	}
	mtx.unlock();
	qDebug() << "table mtx unlocked"; 
}
/*
void Data_TableModel::setModalData(QList< QStringList > *rowlist)
{
arr_row_list = rowlist;
}
*/
void Data_TableModel::refrushModel()
{
	//��ʼģ�����ò�����
	//���ò������κθ�����ͼ�н�ģ������Ϊ�䵱ǰ״̬��
	//��һ��ģ�ͱ�����ʱ������ζ����ǰ��ģ���б�����κ��������ڶ�����Ч�ģ������ٴβ�ѯ��
	//��Ҳ��ζ�ŵ�ǰitem���κ�ѡ�е�item������Ч��
	//��һ��ģ�ʹӸ����ϸı���������ʱ����ʱ�����������Ҫ�ȷ���dataChanged()��֪ͨ��������ײ�����Դ����ṹ�ı仯Ҫ���׵öࡣ
	//����������ģ�ͻ����ģ���е��κ��ڲ����ݽṹ֮ǰ��������ô˺�����
	beginResetModel();

	//���ģ�����ò�����
	//����������������ģ�ͻ����ģ���е��κ��ڲ����ݽṹ����ô˺�����
	//�˺��������ź�modelReset()��
	endResetModel();

	emit updateCount(this->rowCount(QModelIndex()));
}
