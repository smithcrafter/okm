/*
 * Licensed under the MIT License <http://opensource.org/licenses/MIT>.
 * Copyright (C) 2016-2023 Vladimir Kuznetsov <smithcoder@yandex.ru> https://smithcoder.ru
 */

#pragma once

#include <cstdlib>
#include <memory.h>
#include <utility>

#ifndef DWLOG
#define DWLOG(text)
#define TEMPORATY_DWLOG_DISABLED
#endif

#define BASESIZE 6400

namespace Smitto {

enum class FindAlgorithm
{
	BinarySeparation,
	RelativePrediction
};


template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM = FindAlgorithm::BinarySeparation>
class OrderedKeyMap
{
public:
	struct Pair
	{
		KTYPE key;
		TYPE value;
		Pair(KTYPE pkey, TYPE&& pvalue) : key(pkey), value(std::move(pvalue)) {}
		Pair(KTYPE pkey, const TYPE& pvalue) : key(pkey), value(pvalue) {}
	};
	struct iterator
	{
		iterator() : container_(nullptr), pos_(0) {}
		iterator(const OrderedKeyMap* container, int ppos) : container_(container), pos_(ppos) {}
		inline KTYPE key() const {if (pos_ >= container_->size() || pos_ < 0) return -1; return container_->dataAt(pos_).key;}
		inline TYPE& value() {return container_->dataAt(pos_).value;}
		inline TYPE value() const {return container_->dataAt(pos_).value;}
		inline int pos() const {return pos_;}
		inline bool operator != (const iterator& other) const {return pos_ != other.pos_;}
		inline bool operator == (const iterator& other) const {return pos_ == other.pos_;}
		inline iterator& operator ++ () {pos_++; return *this;}
		inline iterator operator++(int) {iterator r = *this; pos_++; return r;}
		inline iterator& operator -- () {pos_--; return *this;}
		inline iterator operator --(int) {iterator r = *this; pos_--; return r;}
		inline iterator operator + (int n) const {return iterator(container_, pos_+n);}
		inline iterator operator - (int n) const {return iterator(container_, pos_-n);}
		inline TYPE& operator*() {return value();}
		inline TYPE* operator->() {return &value();}
		inline operator bool() const  {return pos_ >= 0 && pos_ < container_->count();}
		bool isEnd() const {return pos_ == container_->count();}
	private:
		const OrderedKeyMap* container_ = nullptr;
		int pos_ = 0;
	};

// standard
	inline TYPE operator [](KTYPE key) const {auto it = find(key); if (it != constEnd()) return it.value();
		DWLOG(name + QString(" OKM: Miss - key %1. Range %2-%3 count %4").arg(key).arg(firstKey_).arg(lastKey_).arg(count_));
		return emptyVal;}
	TYPE& operator [](KTYPE key);
	inline TYPE value(KTYPE key) const {return const_cast<OrderedKeyMap*>(this)->operator[] (key);}
	inline TYPE& first() {if (count_) return (*(Pair*)((char*)data_)).value;
		DWLOG(name + " OKM: Miss - first"); return emptyVal;}
	inline TYPE first() const {return const_cast<OrderedKeyMap*>(this)->first(); }
	inline TYPE& last() {if (count_) return (*(Pair*)((char*)data_+(count_-1)*sizeof(Pair))).value;
		DWLOG(name + " OKM: Miss - last"); return emptyVal;}
	inline TYPE last() const {return const_cast<OrderedKeyMap*>(this)->last();}
	inline KTYPE lastKey() const {return lastKey_;}
	inline KTYPE firstKey() const {return firstKey_;}
	inline bool contains(KTYPE key) const { return constFind(key) != constEnd(); }
	inline int count() const {return count_;}
	inline int size() const {return count_;}
	inline bool isEmpty() const {return !count_;}
	inline bool empty() const {return isEmpty();}
	iterator insert(KTYPE key, TYPE value);
	void remove(KTYPE key);
	inline void clear() {count_ = 0; lastKey_ = 0; firstKey_ = 0;}

// additional
	TYPE& valueNearPos(KTYPE key, int pos);
	TYPE valueNearPos(KTYPE key, int pos) const {return const_cast<OrderedKeyMap*>(this)->valueNearPos(key, pos);}
	inline Pair& dataAt(int pos) const {return *(Pair*)((char*)data_+pos*sizeof(Pair));}
	bool equal(const OrderedKeyMap& other) const;
#ifdef QMAP_H
	bool equal(const QMap<KTYPE, TYPE>& other) const;
#endif
#ifdef QLIST_H
	QList<KTYPE> keys() const;
	QList<KTYPE> keys(KTYPE min, KTYPE max) const;
	QList<TYPE> values() const;
#endif
#ifdef QPAIR_H
	QPair<KTYPE, KTYPE> interval() const {return qMakePair(firstKey_, lastKey_);}
#endif

	void trimAfter(KTYPE key) {auto it = lowerBound(key); if (it == constBegin()) return; count_ = it.pos()+1; lastKey_ = it.key();}

// iterators
	typedef iterator Iterator;
	typedef iterator ConstIterator;
	inline iterator begin() const {return constBegin();}
	inline iterator end() const {return constEnd();}
	inline iterator at(int pos) const {return iterator(this, pos < count_ ? pos : count_);}
	inline iterator constBegin() const {return iterator(this, 0);}
	inline iterator constEnd() const {return iterator(this, count_);}
	iterator find(KTYPE key);
	iterator findAlt(KTYPE key);
	inline iterator find (KTYPE key) const {return const_cast<OrderedKeyMap*>(this)->find(key);}
	inline iterator constFind (KTYPE key) const {return find(key);}
	iterator lowerBound(KTYPE key) const;
	iterator upperBound(KTYPE key) const {auto it = lowerBound(key); if (constEnd() == it || key < it.key()) return it; return ++it;}
	iterator upperBoundAlt(KTYPE key) const;

// constructors
	OrderedKeyMap(int size = BASESIZE) {if (size > 0) reserveData(size*sizeof(Pair));}
	OrderedKeyMap(const OrderedKeyMap& o) {
		reserveData(o.dataSize_); memcpy(data_, o.data_, dataSize_); count_ = o.count_;
		lastKey_ = o.lastKey_; firstKey_ = o.firstKey_; }
	OrderedKeyMap(OrderedKeyMap&& o) noexcept {
		dataSize_= o.dataSize_; data_ = o.data_; lastKey_ = o.lastKey_; firstKey_ = o.firstKey_; count_ = o.count_;
		o.data_ = nullptr; o.dataSize_ = 0; o.lastKey_ = 0; o.firstKey_ = 0; o.count_ = 0; }
	OrderedKeyMap(const void* data, int dataSize) {
		reserveData(dataSize); memcpy(data_, data, dataSize_ = dataSize); count_ = dataSize/sizeof(Pair);
		if (count_) {firstKey_ = at(0).key(); lastKey_ = at(count_-1).key();} }
	~OrderedKeyMap() {dealoc();}


	static OrderedKeyMap fromRawData(const void* data, int dataSize) { OrderedKeyMap res(0);
		res.dataSize_ = 0; res.data_ = const_cast<void*>(data); res.count_ = dataSize/sizeof(Pair);
		if (res.count_) {res.firstKey_ = res.at(0).key(); res.lastKey_ = res.at(res.count_-1).key();} return res;}

// operators
	OrderedKeyMap& operator = (OrderedKeyMap&& o) noexcept {
		dealoc(); dataSize_= o.dataSize_; data_ = o.data_;
		lastKey_ = o.lastKey_; firstKey_ = o.firstKey_;  count_ = o.count_;
		o.data_ = nullptr; o.dataSize_ = 0; o.lastKey_ = 0; o.firstKey_ = 0; o.count_ = 0; return *this;}
	OrderedKeyMap& operator = (const OrderedKeyMap& o) {
		if (dataSize_ < o.dataSize_)  {dealoc(); reserveData(o.dataSize_); }
		memcpy(data_, o.data_, o.dataSize_); count_ = o.count_;
		lastKey_ = o.lastKey_; firstKey_ = o.firstKey_; return *this;}
	inline bool operator == (const OrderedKeyMap& o) const {return count_ == o.count_
				&& firstKey_ == o.firstKey_ && lastKey_ == o.lastKey_
				&& memcmp(data_, o.data_, count_*sizeof(Pair)) == 0;}


	OrderedKeyMap mid(KTYPE from, KTYPE to, int reserve = 0) const {
		auto itStart = lowerBound(from);
		auto itEnd = lowerBound(to);
		if (itStart == end() || itEnd.pos() < itStart.pos())
			return OrderedKeyMap(0);
		if (itEnd == end())
			--itEnd;
		int count = itEnd.pos() - itStart.pos()+1;
		OrderedKeyMap res(count+reserve);
		memcpy(res.data_, (char*)data_+itStart.pos()*sizeof(Pair), count*sizeof(Pair));
		res.firstKey_ = itStart.key();
		res.lastKey_ = itEnd.key();
		res.count_ = count;
		return res;
	}
	bool insertAtBegining(const OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>& other);
	bool insertAfterEnd(const OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>& other);

#ifdef QSTRING_H
	QString name;
	OrderedKeyMap(const QString& nameArg, int size = BASESIZE) : OrderedKeyMap(size) {name = nameArg;}
#endif
#ifdef QBYTEARRAY_H
	explicit OrderedKeyMap(const QByteArray& ba) : OrderedKeyMap(ba.data(), ba.size()) {}
	QByteArray toRawDataByteArray() const {return QByteArray::fromRawData((const char*)data(), dataSize());}
#endif
	int dataSize() const {return count_*sizeof(Pair);}
	const void* data() const {return data_;}
	void reserve(int k) {if (k*int(sizeof(Pair)) > dataSize_) realoc(k-dataSize_/sizeof(Pair));}

private:
	void reserveData(int k) {if (k > 0) data_ = malloc(dataSize_ = k);}
	void realoc(int addk) {void *ldata = data_; reserveData(dataSize_+addk); memcpy(data_, ldata, dataSize_-addk); free(ldata);}
	void dealoc() {clear(); if (data_ && dataSize_) free(data_); data_ = nullptr; dataSize_ = 0;}
	TYPE& insertBefore(int pos, KTYPE key, TYPE&& value);

private:
	int dataSize_ = 0;
	void* data_ = nullptr;
	int count_ = 0;
	KTYPE lastKey_ = 0;
	KTYPE firstKey_ = 0;
	TYPE emptyVal = TYPE(); // 0
};

template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
typename OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::iterator OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::insert(KTYPE key, TYPE value)
{
	if (!dataSize_)
		reserveData(BASESIZE*sizeof(Pair));
	if (empty())
	{
		new ((Pair*)((char*)data_)) Pair(key, std::move(value));
		count_++;
		firstKey_ = key;
		lastKey_ = key;
		return constBegin();
	}
	if (key > lastKey_)
	{
		if (int((count_+1)*sizeof(Pair)) > dataSize_)
			realoc(dataSize_);
		new ((Pair*)((char*)data_+count_*sizeof(Pair))) Pair(key, std::move(value));
		lastKey_ = key;
		return iterator(this, count_++);
	}
	auto it = lowerBound(key);
	if (it.key() == key)
	{
#ifdef DEBUG
		if (it.pos() != count_-1)
		{
			DWLOG(name + QString(" OKM: Вставка в середину %1 из %2").arg(it.pos()).arg(count_));
		}
#endif
		((Pair*)((char*)data_+it.pos()*sizeof(Pair)))->value = value;
		return it;
	}
	insertBefore(it.pos(), key, std::move(value));
	return it;
}

template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
TYPE& OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::insertBefore(int pos, KTYPE key, TYPE&& value)
{
	if (int((count_+1)*sizeof(Pair)) > dataSize_)
	{
		void *ldata = data_;
		reserveData(2*dataSize_);
		if (pos > 0)
			memcpy((char*)data_, ldata, pos*sizeof(Pair));
		DWLOG(name + (pos > 0 ? QString("OKM: Inserting element %1 in the middle and increasing the size").arg(key) :
							 QString("Inserting element %1 at the beginning and increasing the size").arg(key)));
		memcpy((char*)data_+(pos+1)*sizeof(Pair), (char*)ldata+(pos)*sizeof(Pair),
			   (count_ - pos)*sizeof(Pair));
		free(ldata);
	}
	else
	{
		DWLOG(name + (pos > 0 ? QString("OKM: Inserting element %1 in the middle is highly discouraged").arg(key) :
							 QString("Inserting element %1 at the beginning is highly discouraged").arg(key)));
		memmove((char*)data_+(pos+1)*sizeof(Pair), (char*)data_+(pos)*sizeof(Pair),
				(count_-pos)*sizeof(Pair));
	}
	Pair* pair = (Pair*)((char*)data_+pos*sizeof(Pair));
	*pair = Pair(key, std::move(value));
	if (pos == 0)
		firstKey_ = key;
	count_++;
	return pair->value;
}

template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
bool OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::insertAtBegining(const OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>& other)
{
	if (other.lastKey() >= firstKey())
		return false;
	void *ldata = data_;
	reserveData((other.count_ + count_ + BASESIZE)*sizeof(Pair));
	memcpy((char*)data_, other.data_, other.count_*sizeof(Pair));
	if (empty())
		lastKey_ = other.lastKey_;
	else
		memcpy((char*)data_+other.count_*sizeof(Pair), ldata, count_*sizeof(Pair));
	count_ = other.count_ + count_;
	firstKey_ = other.firstKey_;
	free(ldata);
	return true;
}

template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
bool OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::insertAfterEnd(const OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>& other)
{
	if (other.firstKey() <= lastKey())
		return false;
	if ((other.count_ + count_)*int(sizeof(Pair)) > dataSize_)
		realoc((other.count_ + count_ + BASESIZE)*sizeof(Pair));
	memcpy((char*)data_+count_*sizeof(Pair), other.data_, other.count_*sizeof(Pair));
	if (empty())
		firstKey_ = other.firstKey_;
	count_ = other.count_ + count_;
	lastKey_ = other.lastKey_;
	return true;
}

template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
void OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::remove(KTYPE key)
{
	if (key == lastKey_)
	{
		if (--count_)
			lastKey_ = dataAt(count_ - 1).key;
		else
		{
			lastKey_ = 0;
			firstKey_ = 0;
		}
		return;
	}
	DWLOG(name + QString("OKM: Removing element of element %1 from the middle is highly discouraged").arg(key));
	auto it = lowerBound(key);
	if (it == constEnd())
		return;
	memmove((char*)data_+(it.pos()-1)*sizeof(Pair), (char*)data_+(it.pos())*sizeof(Pair), count_-it.pos());
	count_--;
}

template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
bool OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::equal(const OrderedKeyMap& o) const
{
	if (count_ != o.count() || firstKey_ != o.firstKey_ || lastKey_ != o.lastKey_)
		return false;
	if (sizeof(TYPE) % 8 == 0)
		return memcmp(data_, o.data_, o.count_*sizeof(Pair)) == 0;
	auto itc = constBegin();
	for (auto it = o.constBegin(); it != o.constEnd(); ++it, ++itc)
		if (it.key() != itc.key() || it.value() != itc.value())
			return false;
	return true;
}

#ifdef QMAP_H
template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
bool OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::equal(const QMap<KTYPE, TYPE>& o) const
{
	if (count_ != o.count() || firstKey_ != o.firstKey() || lastKey_ != o.lastKey())
		return false;
	auto itc = constBegin();
	for (auto it = o.constBegin(); it != o.constEnd(); ++it, ++itc)
		if (it.key() != itc.key() || it.value() != itc.value())
			return false;
	return true;
}
#endif

template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
TYPE& OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::valueNearPos(KTYPE key, int pos)
{
	if (pos < count_ && pos >= 0)
	{
		auto& data = dataAt(pos);
		if (data.key == key)
			return data.value;
		int p = key > data.key ? 1 : -1;
		int pos2 = pos;
		do
		{
			pos2 += p;
			auto& data2 = dataAt(pos2);
			if (data.key == key)
			{
				DWLOG(name + QString("OKM: Miss - key %1 pos %2 pos2 %3").arg(key).arg(pos).arg(pos2));
				return data2.value;
			}
			else if (p*(data2.key - key) > 0)
				break;

		} while(pos2 >= 0 && pos2 < count_);
	}
	DWLOG(name + "OKM: Miss - valueNearPos");
	return emptyVal;
}

enum class SearchType
{
	LowerBound,
	UpperBound,
	Find
};

template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM = FindAlgorithm::BinarySeparation>
static inline typename OrderedKeyMap<KTYPE, TYPE, FindAlgorithm::BinarySeparation>::iterator internalSearch(
		const OrderedKeyMap<KTYPE, TYPE, FindAlgorithm::BinarySeparation>& container, KTYPE key, SearchType stype)
{
	int begin = 0, end = container.count()-1;
	while (begin + 1 < end)
	{
		auto pos = (end+begin)/2;
		auto& atval = container.dataAt(pos);
		if (atval.key == key)
		{
			if (stype == SearchType::UpperBound)
				pos++;
			return typename OrderedKeyMap<KTYPE, TYPE, FindAlgorithm::BinarySeparation>::iterator(&container, pos);
		}
		key > atval.key ? begin = pos : end = pos;
	}
	if (stype == SearchType::LowerBound || stype == SearchType::UpperBound)
		return typename OrderedKeyMap<KTYPE, TYPE, FindAlgorithm::BinarySeparation>::iterator(&container, end);
	return container.constEnd();
}

template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM = FindAlgorithm::RelativePrediction>
static inline typename OrderedKeyMap<KTYPE, TYPE, FindAlgorithm::RelativePrediction>::iterator internalSearch(
		const OrderedKeyMap<KTYPE, TYPE, FindAlgorithm::RelativePrediction>& container, KTYPE key, SearchType stype)
{
	int begin = 0,  end = container.count()-1;
	KTYPE beginKey = container.firstKey(), endKey = container.lastKey();
	while (begin + 1 < end)
	{
		int pos = begin + (end-begin)*float(key-beginKey)/(endKey-beginKey);
		if (pos <= begin)
			pos = begin+1;
		else if (pos >= end)
			pos = end-1;
		auto& atval = container.dataAt(pos);
		if (atval.key == key)
		{
			if (stype == SearchType::UpperBound)
				pos++;
			return typename OrderedKeyMap<KTYPE, TYPE, FindAlgorithm::RelativePrediction>::iterator(&container, pos);
		}
		if (key > atval.key)
		{
			begin = pos;
			beginKey = atval.key;
		}
		else
		{
			end = pos;
			endKey = atval.key;
		}
	}
	if (stype == SearchType::LowerBound || stype == SearchType::UpperBound)
		return typename OrderedKeyMap<KTYPE, TYPE, FindAlgorithm::RelativePrediction>::iterator(&container, end);
	return container.constEnd();
}

template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
typename OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::iterator OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::lowerBound(KTYPE key) const
{
	if (empty() || key > lastKey_)
		return constEnd();
	if (key == lastKey_)
		return iterator(this, count_-1);
	if (key <= firstKey_)
		return iterator(this, 0);
	return internalSearch<KTYPE, TYPE, FINDALGORITHM>(*this, key, SearchType::LowerBound);
}

template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
typename OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::iterator OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::upperBoundAlt(KTYPE key) const
{
	if (empty() || key >= lastKey_)
		return constEnd();
	if (key == firstKey_)
		return iterator(this, 1);
	if (key < firstKey_)
		return iterator(this, 0);
	return internalSearch<KTYPE, TYPE, FINDALGORITHM>(*this, key, SearchType::UpperBound);
}


template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
TYPE& OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::operator [](KTYPE key)
{
	if (key == lastKey_)
		return last();
	if (key > lastKey_)
		return this->insert(key, TYPE()).value();
	auto it = lowerBound(key);
	if (it.key() != key)
		return this->insertBefore(it.pos(), key, TYPE());
	return it.value();
}

template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
typename OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::iterator OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::find(KTYPE key)
{
	auto it = lowerBound(key);
	if (it == constEnd() || it.key() == key)
		return it;
	return constEnd();
}

template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
typename OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::iterator OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::findAlt(KTYPE key)
{
	if (empty() || key > lastKey_ || key < firstKey_)
		return constEnd();
	if (key == lastKey_)
		return iterator(this, count_-1);
	if (key == firstKey_)
		return iterator(this, 0);
	return internalSearch<KTYPE, TYPE, FINDALGORITHM>(*this, key, SearchType::Find);
}

#ifdef QLIST_H
template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
QList<KTYPE> OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::keys() const
{
	QList<KTYPE> res;
	for (auto it = constBegin(); it != constEnd(); ++it)
		res.append(it.key());
	return res;
}
template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
QList<KTYPE> OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::keys(KTYPE min, KTYPE max) const
{
	QList<KTYPE> res;
	auto itEnd = max ? upperBound(max) : constEnd();
	for (auto it = lowerBound(min); it != itEnd; ++it)
		res.append(it.key());
	return res;
}
template <typename KTYPE, typename TYPE, FindAlgorithm FINDALGORITHM>
QList<TYPE> OrderedKeyMap<KTYPE, TYPE, FINDALGORITHM>::values() const
{
	QList<TYPE> res;
	for (auto it = constBegin(); it != constEnd(); ++it)
		res.append(it.value());
	return res;
}
#endif

} // Smitto::

#ifdef TEMPORATY_DWLOG_DISABLED
#undef DWLOG
#undef TEMPORATY_DWLOG_DISABLED
#endif
