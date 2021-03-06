#include "data_tablemodel.h"
#include "global_buffer.h"

Data_TableModel::Data_TableModel(QObject *parent)
	: QAbstractTableModel(parent)
{
	setHorizontalHeaderList(QStringList() << QStringLiteral("ID") 
										  << QStringLiteral("Time stamp")
										  << QStringLiteral("Object_DistLong") 
										  << QStringLiteral("Object_DistLat"));
}

Data_TableModel::~Data_TableModel(void)
{
	arr_row_list.clear();
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

	if (arr_row_list.empty())
		return 0;
	else
		return arr_row_list.size();
}

//Returns the number of columns for the children of the given parent.
int Data_TableModel::columnCount(const QModelIndex &parent) const
{
	if (horizontal_header_list.size() > 0)
		return horizontal_header_list.size();

	if (arr_row_list.empty())
		return 0;
	else if (arr_row_list.size() < 1)
		return 0;
	else
		return arr_row_list.begin().value().size()+1;
}

//Returns the data stored under the given role for the item referred to by the index.
QVariant Data_TableModel::data(const QModelIndex &index, int role) const
{
	//Qmodelindex用作从QAbstractItemModel派生的item索引。
	//项目视图、委托和选择模型需要使用索引来定位模型中的项目。
	if (!index.isValid()) {
		return QVariant();
	}

	if (arr_row_list.empty()) {
		return QVariant();
	}

	if (arr_row_list.size() < 1) {
		return QVariant();
	}

	//使用默认委托展示的item的文本对齐方式
	if (role == Qt::TextAlignmentRole) {
		return int(Qt::AlignLeft | Qt::AlignVCenter);
	}

	//以文本形式显示的核心数据
	else if (role == Qt::DisplayRole) {
		if (index.row() >= arr_row_list.size())
			return QVariant();
		if (index.column() >= arr_row_list.begin().value().size() + 1)
			return QVariant();
		if (arr_row_list.empty())
			return QVariant();
		if(index.column() == 0)
			return (arr_row_list.begin()+index.row()).key();
		return (arr_row_list.begin() + index.row()).value()[index.column() - 1];
	}
	return QVariant();
}

//Returns the data for the given role and section in the header with the specified orientation(横向或者竖向).
QVariant Data_TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole) {
		if (orientation == Qt::Horizontal) // 水平表头
		{
			if (horizontal_header_list.size() > section)
				return horizontal_header_list[section];
			else
				return QVariant();
		}
		else {
			if (vertical_header_list.size() > section)
				return vertical_header_list[section]; // 垂直表头
			else
				return QVariant();
		}
	}

	return QVariant();
}

//供其他组件使用，以获取关于模型提供的每个item的信息。
Qt::ItemFlags Data_TableModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::NoItemFlags;

	Qt::ItemFlags flag = QAbstractItemModel::flags(index);

	// flag|=Qt::ItemIsEditable // 设置单元格可编辑,此处注释,单元格无法被编辑  
	return flag;
}
void Data_TableModel::refresh_with_new_frame(QString time_stamp)
{
	qDebug() << "time_stamp: " << time_stamp;
	arr_row_list.clear();
	std::map<QString,int> needed_signame={
		{"Obj_ID",0},{"Obj_DistLong",1},{"Obj_DistLat",2}
	};
	
	mtx.lock();
	qDebug() << "table mtx locked";
	DBCMessage msg;
	auto cur_info = global_buffer.takeFirst();
	qDebug() << "data length: " << cur_info.vco[0].DataLen;
	for (int i = 0; i < cur_info.noframe; ++i) {

		//一个 msg/vco[i] 对应一个 piece
		if (DBC_Analyse(cfg->m_hDBC, &cur_info.vco[i], &msg)) {
			
			//qDebug() << "analyse success";
			//qDebug() << msg.strName;

			if (strcmp( msg.strName, "Obj_1_General") == 0) {
				QList<QString> _list = { time_stamp, "", "" };
				unsigned int _ID = 0;
				qDebug() << "we are in general";
				for (int j = 0; j < msg.nSignalCount; j++) {
					auto _name = msg.vSignals[j].strName;
					qDebug() << _name;

					//if signal name is Obj_ID
					if (needed_signame.find(_name) != needed_signame.end() 
						&& needed_signame.find(_name)->second == 0) {
						qDebug() << needed_signame.at(_name) << ": " << msg.vSignals[j].nValue;
						_ID = msg.vSignals[j].nValue;
					}
					else if (needed_signame.find(_name) != needed_signame.end()) {
						_list[needed_signame.at(_name)] = QString::number(msg.vSignals[j].nValue);
					}
				}
				//std::pair<int, QList<QString>> _piece(_ID, _list);
				arr_row_list.insert(_ID, _list);
			}
		}
	}
	refrushModel();
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
	//开始模型重置操作。
	//重置操作在任何附加视图中将模型重置为其当前状态。
	//当一个模型被重置时，这意味着以前从模型中报告的任何数据现在都是无效的，必须再次查询。
	//这也意味着当前item和任何选中的item都将无效。
	//当一个模型从根本上改变它的数据时，有时调用这个函数要比发出dataChanged()来通知其他组件底层数据源或其结构的变化要容易得多。
	//在重新设置模型或代理模型中的任何内部数据结构之前，必须调用此函数。
	beginResetModel();

	//完成模型重置操作。
	//您必须在重新设置模型或代理模型中的任何内部数据结构后调用此函数。
	//此函数发出信号modelReset()。
	endResetModel();

	emit updateCount(this->rowCount(QModelIndex()));
}

void Data_TableModel::slot_disconnect() {
	arr_row_list.clear();
	refrushModel();
}
