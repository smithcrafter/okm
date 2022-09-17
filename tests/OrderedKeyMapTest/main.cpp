/*
 * Licensed under the MIT License <http://opensource.org/licenses/MIT>.
 * Copyright (C) 2016-2022 Vladimir Kuznetsov <smithcoder@yandex.ru> https://smithcoder.ru
 */

#include <QElapsedTimer>
#include <QDateTime>
#include <QDebug>
#include <QList>
#include <map>
#include <unordered_map>
#include <smitto/okm.h>

struct TestValue
{
	quint64 val;
	TestValue() = default;
	TestValue(quint32 pval) : val(pval) {}
	TestValue& operator +=(const TestValue& o) {val+=o.val; return *this;}
};
QDebug operator<< (QDebug dbg, const TestValue& v) {dbg.nospace() << v.val;return dbg.space();}

auto maxCount = 10'000'000;
auto maxRandKeys = 1'000'000;
using KeyType = quint32;
using ValueType = TestValue;

int main(int argc, char *argv[])
{
	Q_UNUSED(argc);
	Q_UNUSED(argv)
	std::srand(QDateTime::currentDateTime().toMSecsSinceEpoch());

	Smitto::OrderedKeyMap<KeyType, ValueType> testmap;

	QMap<KeyType, ValueType> qt_map;
	std::unordered_map<KeyType, ValueType> std_umap;
	std::map<KeyType, ValueType> std_map;
	Smitto::OrderedKeyMap<KeyType, ValueType, Smitto::FindAlgorithm::BinarySeparation> s_okm_0;
	Smitto::OrderedKeyMap<KeyType, ValueType, Smitto::FindAlgorithm::RelativePrediction> s_okm_1;

	auto end = QDateTime(QDate::currentDate().addDays(100*365), QTime(0,0,0)).toSecsSinceEpoch();
	for (qint64 i = QDateTime(QDate::currentDate(), QTime(0,0,0)).toSecsSinceEpoch(); i < end && testmap.size() < maxCount; i+=60)
		if (quint32 tod = i%(3600*24))
			if (tod > 7*3600 && tod < 20*3600)
				testmap.insert(i, std::rand());
	qDebug()<<"Count:"<<testmap.size()<<"random count"<<maxRandKeys<<"KEY:"<<typeid(KeyType).name()<<"VALUE:"<<typeid(ValueType).name();
	QElapsedTimer timer;

	qDebug()<<"---INSERT---";

	timer.start();
	for (auto it = testmap.constBegin(); it != testmap.constEnd(); ++it)
		qt_map.insert(it.key(), it.value());
	qDebug()<<"qt_map   insert count="<<qt_map.size()<<"time:"<<timer.nsecsElapsed()<<"ns";

	timer.restart();
	for (auto it = testmap.constBegin(); it != testmap.constEnd(); ++it)
		std_umap.insert(std::pair<KeyType, ValueType>(it.key(), it.value()));
	qDebug()<<"std_umap insert count="<<std_umap.size()<<"time:"<<timer.nsecsElapsed()<<"ns";

	timer.restart();
	for (auto it = testmap.constBegin(); it != testmap.constEnd(); ++it)
		std_map.insert(std::pair<KeyType, ValueType>(it.key(), it.value()));
	qDebug()<<"std_map  insert count="<<std_map.size()<<"time:"<<timer.nsecsElapsed()<<"ns";

	timer.restart();
	for (auto it = testmap.constBegin(); it != testmap.constEnd(); ++it)
		s_okm_0.insert(it.key(), it.value());
	qDebug()<<"s_okm_0  insert count="<<s_okm_0.count()<<"time:"<<timer.nsecsElapsed()<<"ns";

	timer.restart();
	for (auto it = testmap.constBegin(); it != testmap.constEnd(); ++it)
		s_okm_1.insert(it.key(), it.value());
	qDebug()<<"s_okm_1  insert count="<<s_okm_1.count()<<"time:"<<timer.nsecsElapsed()<<"ns";

/// --------------------------------------------------------------------
	qDebug()<<"---CIRCLE FOR BY ITERATOR---";
	ValueType sum = 0; timer.restart();
	for (auto it = qt_map.constBegin(); it != qt_map.constEnd(); ++it)
		sum+= it.value();
	qDebug()<<"qt_map   for iterator sum="<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	timer.restart(); sum = 0;
	for (auto it = std_umap.begin(); it != std_umap.end(); ++it)
		sum+= it->second;
	qDebug()<<"std_umap for iterator sum="<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	timer.restart(); sum = 0;
	for (auto it = std_map.begin(); it != std_map.end(); ++it)
		sum+= it->second;
	qDebug()<<"std_map  for iterator sum="<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	timer.restart(); sum = 0;
	for (auto it = s_okm_0.constBegin(); it != s_okm_0.constEnd(); ++it)
		sum += it.value();
	qDebug()<<"s_okm_0  for iterator sum="<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	timer.restart(); sum = 0;
	for (auto it = s_okm_1.constBegin(); it != s_okm_1.constEnd(); ++it)
		sum += it.value();
	qDebug()<<"s_okm_1  for iterator sum="<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";


	auto tkeys = testmap.keys();
	QVector<KeyType> randoms;
	randoms.reserve(maxRandKeys);
	for (int i = 0; i < maxRandKeys; i++)
		randoms.append(tkeys[std::rand()%testmap.size()]);

/// --------------------------------------------------------------------
	qDebug()<<"---CIRCLE FOR BY ORDERED TKEYS---";

	sum = 0; timer.restart();
	for(auto tkey : tkeys)
		sum += qt_map[tkey];
	qDebug()<<"qt_map   operator[] order_pass sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : tkeys)
		sum += std_umap[tkey];
	qDebug()<<"std_umap operator[] order_pass sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : tkeys)
		sum += std_map[tkey];
	qDebug()<<"std_map  operator[] order_pass sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : tkeys)
		sum += s_okm_0[tkey];
	qDebug()<<"s_okm_0  operator[] order_pass sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : tkeys)
		sum += s_okm_1[tkey];
	qDebug()<<"s_okm_1  operator[] order_pass sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";


/// --------------------------------------------------------------------
	qDebug()<<"--- FIND BY ORDERED TKEYS---";
	sum = 0; timer.restart();
	for(auto tkey : tkeys)
		sum += qt_map.find(tkey).value();
	qDebug()<<"qt_map   tkeys find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : tkeys)
		sum += std_umap.find(tkey)->second;
	qDebug()<<"std_umap tkeys find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : tkeys)
		sum += std_map.find(tkey)->second;
	qDebug()<<"std_map  tkeys find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : tkeys)
		sum += s_okm_0.find(tkey).value();
	qDebug()<<"s_okm_0  tkeys find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : tkeys)
		sum += s_okm_1.find(tkey).value();
	qDebug()<<"s_okm_1  tkeys find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : tkeys)
		sum += s_okm_0.findAlt(tkey).value();
	qDebug()<<"s_okm_0  tkeys find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns Alt";

	sum = 0; timer.restart();
	for(auto tkey : tkeys)
		sum += s_okm_1.findAlt(tkey).value();
	qDebug()<<"s_okm_1  tkeys find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns Alt";


/// --------------------------------------------------------------------
	qDebug()<<"---FIND BY RANDOM KEYS---";
	sum = 0; timer.restart();
	for(auto tkey : randoms)
		sum += qt_map.find(tkey).value();
	qDebug()<<"qt_map   key randoms find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : randoms)
		sum += std_umap.find(tkey)->second;
	qDebug()<<"std_umap key randoms find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : randoms)
		sum += std_map.find(tkey)->second;
	qDebug()<<"std_map  key randoms find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_0.find(tkey).value();
	qDebug()<<"s_okm_0  key randoms find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_1.find(tkey).value();
	qDebug()<<"s_okm_1  key randoms find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_0.findAlt(tkey).value();
	qDebug()<<"s_okm_0  key randoms find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns Alt";

	sum = 0; timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_1.findAlt(tkey).value();
	qDebug()<<"s_okm_1  key randoms find sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns Alt";


/// --------------------------------------------------------------------
	qDebug()<<"---OPERATOR[] BY RANDOM KEYS---";
	sum = 0; timer.restart();
	for(auto tkey : randoms)
		sum += qt_map[tkey];
	qDebug()<<"qt_map   operator[] random_pass sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += std_umap[tkey];
	qDebug()<<"std_umap operator[] random_pass sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += std_map[tkey];
	qDebug()<<"std_map  operator[] random_pass sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_0[tkey];
	qDebug()<<"s_okm_0  operator[] random_pass sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_1[tkey];
	qDebug()<<"s_okm_1  operator[] random_pass sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";


/// --------------------------------------------------------------------
	qDebug()<<"---CONTAINS BY BY RANDOM KEYS---";
	sum = 0; timer.restart();
	for(auto tkey : randoms)
		sum += qt_map.contains(tkey);
	qDebug()<<"qt_map   key randoms contains sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += std_umap.contains(tkey);
	qDebug()<<"std_umap key randoms contains sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += std_map.contains(tkey);
	qDebug()<<"std_map  key randoms contains sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_0.contains(tkey);
	qDebug()<<"s_okm_0  key randoms contains sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_1.contains(tkey);
	qDebug()<<"s_okm_1  key randoms contains sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	for (int i = 0; i < 200; i++)
	{
		auto time = end-std::rand();
		if (qt_map.contains(time) != s_okm_0.contains(time))
			qDebug()<<"err contains";
	}

/// --------------------------------------------------------------------
	qDebug()<<"---LOWERBOUND BY RANDOM KEYS---";
	sum = 0; timer.restart();
	for(auto tkey : randoms)
		sum += qt_map.lowerBound(tkey).value();
	qDebug()<<"qt_map   key randoms lowerBound sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += std_map.lower_bound(tkey)->second;
	qDebug()<<"std_map  key randoms lowerBound sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_0.lowerBound(tkey).value();
	qDebug()<<"s_okm_0  key randoms lowerBound sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_1.lowerBound(tkey).value();
	qDebug()<<"s_okm_1  key randoms lowerBound sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";


/// --------------------------------------------------------------------
	qDebug()<<"---UPPERBOUND BY RANDOM KEYS---";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
	{
		auto it = s_okm_0.lowerBound(tkey);
		if (s_okm_0.constEnd() != it)
			++it;
		sum += it.value();
	}
	qDebug()<<"lmap key randoms upperBound over lowerBound sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0; timer.restart();
	for(auto tkey : randoms)
		sum += qt_map.upperBound(tkey).value();
	qDebug()<<"qt_map   key randoms upperBound sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += std_map.upper_bound(tkey)->second;
	qDebug()<<"std_map  key randoms upperBound sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_0.upperBound(tkey).value();
	qDebug()<<"s_okm_0  key randoms upperBound sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_1.upperBound(tkey).value();
	qDebug()<<"s_okm_1  key randoms upperBound sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_0.upperBoundAlt(tkey).value();
	qDebug()<<"s_okm_0  key randoms upperBound sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns Alt";

	sum = 0;timer.restart();
	for(auto tkey : randoms)
		sum += s_okm_1.upperBoundAlt(tkey).value();
	qDebug()<<"s_okm_1  key randoms upperBound sum"<<sum<<"time:"<<timer.nsecsElapsed()<<"ns Alt";

	return 0;
}
