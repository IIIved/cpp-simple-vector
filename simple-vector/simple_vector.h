#pragma once
#include "array_ptr.h"
#include <cassert>
#include <initializer_list>
#include <algorithm>
#include <stdexcept>
#include <functional>
#include <utility>
 
class ReserveProxyObj {
public:
    ReserveProxyObj(size_t capacity) {
        capacity_ = capacity;
    }
    size_t capacity_;
};
 
ReserveProxyObj Reserve(size_t capacity_to_reserve) {
    return ReserveProxyObj(capacity_to_reserve);
}

template <typename Type>
class SimpleVector {
public:
    using ItemsPtr = ArrayPtr<Type>;
    using Iterator = Type*;
    using ConstIterator = const Type*;
 
    SimpleVector() noexcept = default;
 
    // Создаёт вектор из size элементов, инициализированных значением по умолчанию
    explicit SimpleVector(size_t size):
        items_(size),
        size_(size),
        capacity_(size) {
        std::generate(begin(), end(), [](){return Type();});
    }
 
    // Создаёт вектор из size элементов, инициализированных значением value
    SimpleVector(size_t size, const Type& value):
        items_(size),
        size_(size),
        capacity_(size) {
        std::fill(begin(), end(), value);
        
    }
 
    // Создаёт вектор из std::initializer_list
    SimpleVector(std::initializer_list<Type> init)
    : items_(init.size()),
        size_(init.size()),
        capacity_(init.size()) {
        std::move(init.begin(), init.end(), begin());
    }
 
    // Создаёт пустой вектор и резервирует необходимую память
    explicit SimpleVector(ReserveProxyObj capacity)
        : items_(capacity.capacity_),
        size_(0),
        capacity_(capacity.capacity_) {
    }
 
    //Конструктор копирования и оператор присваивания
    SimpleVector(const SimpleVector& other) {
        SimpleVector tmp(other.size_);
        std::copy(other.begin(), other.end(), tmp.begin());
        this->swap(tmp);
    }

    SimpleVector& operator=(const SimpleVector& rhs) {
        if (&rhs != this) {  // оптимизация присваивания вектора самому себе
            if (rhs.IsEmpty()) {
                // Оптимизация для случая присваивания пустого вектора
                Clear();
            } else {
                // Применяем идиому Copy-and-swap
                SimpleVector rhs_copy(rhs);  // может бросить исключение
                swap(rhs_copy);
            }
        }
        return *this;
    }
    
    SimpleVector& operator=(const SimpleVector&& rhs)
    {
        items_ = std::move(rhs.items_);
        size_ = rhs.size_;
        capacity_ = rhs.capacity_;
        rhs.Clear();
        return *this;
    }
 
    //Перемещающий конструктор и оператор присваивания
    SimpleVector(SimpleVector&& other) noexcept : items_(std::move(other.items_)), size_(other.size_), capacity_(other.capacity_)
    {
        other.Clear();
    }
 
    SimpleVector& operator=(SimpleVector&& rhs) noexcept{
        if (items_.Get() != rhs.items_.Get()) {
            Resize(rhs.size_);
            std::move(rhs.begin(), rhs.end(), begin());
        }
        return *this;
    }
 
    // Возвращает количество элементов в массиве
    size_t GetSize() const noexcept {
        return size_;
    }
 
    // Возвращает вместимость массива
    size_t GetCapacity() const noexcept {
        return capacity_;
    }
 
    // Сообщает, пустой ли массив
    bool IsEmpty() const noexcept {
        return (size_ == 0);
    }
 
    // Возвращает ссылку на элемент с индексом index
    Type& operator[](size_t index) noexcept {
        assert(index <= GetSize());
        return items_[index];
    }
 
    // Возвращает константную ссылку на элемент с индексом index
    const Type& operator[](size_t index) const noexcept {
        return items_[index];
    }
 
    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    Type& At(size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range");
        }
        return items_[index];
    }
 
    // Возвращает константную ссылку на элемент с индексом index
    // Выбрасывает исключение std::out_of_range, если index >= size
    const Type& At(size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index is out of range");
        }
        return items_[index];
    }
 
    // Обнуляет размер массива, не изменяя его вместимость
    void Clear() noexcept {
        size_ = 0;
    }
 
    // Изменяет размер массива.
    // При увеличении размера новые элементы получают значение по умолчанию для типа Type
    void Resize(size_t new_size)
    {
        try
        {
            if (size_ <= new_size)
            {
                auto old_size = size_;
                SimpleVector<Type> tmp(new_size);

                if (!IsEmpty())
                {
                    std::move(begin(), end(), tmp.items_.Get());
                }

                size_ = new_size;
                capacity_ = size_ * 2;
                swap(tmp);
                std::generate((begin() + old_size), end(), [](){return Type();});
            }
            else
            {
                size_ = new_size;
            }
        }
        catch (...)
        {
            throw;
        }
    }
 
    // Возвращает итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator begin() noexcept {
        return items_.Get();
    }
 
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    Iterator end() noexcept {
        return items_.Get() + size_;
    }
 
    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator begin() const noexcept {
        return items_.Get();
    }
 
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator end() const noexcept {
        return items_.Get() + size_;
    }
 
    // Возвращает константный итератор на начало массива
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cbegin() const noexcept {
        return items_.Get();
    }
 
    // Возвращает итератор на элемент, следующий за последним
    // Для пустого массива может быть равен (или не равен) nullptr
    ConstIterator cend() const noexcept {
        return items_.Get() + size_;
    }
 
   void PushBack(const Type& item) {
        const size_t new_size = size_ + 1;
        if (new_size > capacity_) {
            const size_t new_capacity = std::max(capacity_ * 2, new_size);

            auto new_items = ReallocateCopy(new_capacity);  // может бросить исключение
            new_items.Get()[size_] = item;                  // может бросить исключение

            capacity_ = new_capacity;
            items_.swap(new_items);
        } else {
            items_.Get()[size_] = item;
        }
        size_ = new_size;
    }
    
    void PushBack(Type&& value) {
        if (capacity_ == size_) {
            ResizeCapacity(size_ == 0 ? 1 : size_ * 2);
        }
        items_[size_++] = std::move(value);
    }

    Iterator Insert(ConstIterator pos, const Type& value) {
        assert(begin() <= pos && pos <= end());
        size_t new_size = size_ + 1;
        size_t new_item_offset = pos - cbegin();
        if (new_size <= capacity_) {  // Вместимость вектора достаточна для вставки элемента
            Iterator mutable_pos = begin() + new_item_offset;

            // Копируем элементы, начиная с последнего, чтобы перенести "хвост" вправо
            std::copy_backward(pos, cend(),
                               end() + 1);  // может выбросить исключение
            *mutable_pos = value;           // может выбросить исключение

        } else {  // Требуется перевыделить память
            size_t new_capacity = std::max(capacity_ * 2, new_size);

            ItemsPtr new_items(new_capacity);  // может выбросить исключение
            Iterator new_items_pos = new_items.Get() + new_item_offset;

            // Копируем элементы, предшествующие вставляемому
            std::copy(cbegin(), pos, new_items.Get());  // может выбросить исключение
            // Вставляем элемент в позицию вставки
            *new_items_pos = value;  // может выбросить исключение
            // Копируем элементы следующие за вставляемым
            std::copy(pos, cend(), new_items_pos + 1);  // может выбросить исключение

            items_.swap(new_items);
            capacity_ = new_capacity;
        }
        size_ = new_size;
        return begin() + new_item_offset;
    }
 
    Iterator Insert(ConstIterator pos, Type&& value) {
        assert(pos >= begin() && pos <= end());
        Iterator it = const_cast<Iterator>(pos);
        if (capacity_ > size_) {
            std::move_backward(std::make_move_iterator(it),
                               std::make_move_iterator(end()),
                               &items_[size_ + 1]);
            *it = std::move(value);
            ++size_;
            return it;
        }
        if (capacity_ == 0) {
            ArrayPtr<Type> tmp_data(1);
            tmp_data[0] = std::move(value);
            items_ = std::move(tmp_data);
            capacity_ = 1;
            size_ = 1;
            return &items_[0];
        }
        size_t new_capacity = size_ * 2;
        size_t insert_index = std::distance(begin(), it);
        ArrayPtr<Type> tmp_data(new_capacity);
        std::move(std::make_move_iterator(begin()),
                  std::make_move_iterator(it),
                  &tmp_data[0]);
        tmp_data[insert_index] = std::move(value);
        std::move(std::make_move_iterator(it),
                  std::make_move_iterator(end()),
                  &tmp_data[insert_index + 1]);
        items_ = std::move(tmp_data);
        capacity_ = new_capacity;
        ++size_;
        return &items_[insert_index];
    }
 
    // "Удаляет" последний элемент вектора. Вектор не должен быть пустым
    void PopBack() noexcept {
        if (size_ > 0) {
            --size_;
        }
    }
 
    // Удаляет элемент вектора в указанной позиции
    Iterator Erase(ConstIterator pos) {
        auto distance = pos - cbegin();
        Iterator pos_ = items_.Get() + distance;
        std::move(pos_ + 1, end(), pos_);
        --size_;
        return pos_;
    }
 
    // Обменивает значение с другим вектором
    void swap(SimpleVector& other) noexcept {
        items_.swap(other.items_);
        std::swap(size_, other.size_);
        std::swap(capacity_, other.capacity_);
    }
 
    void Reserve(size_t new_capacity) {
        if (new_capacity > capacity_) {
            SimpleVector new_vector(new_capacity);
            std::move(begin(), end(), new_vector.begin());
            items_.swap(new_vector.items_);
            std::exchange(capacity_, new_capacity);
        }
        else return;
    }
 
private:

void ResizeCapacity(size_t new_capacity) {
        ArrayPtr<Type> tmp_data(new_capacity);
        std::move(std::make_move_iterator(begin()),
                  std::make_move_iterator(end()), &tmp_data[0]);
        items_ = std::move(tmp_data);
        capacity_ = new_capacity;
    }

ItemsPtr ReallocateCopy(size_t new_capacity) const {
        ItemsPtr new_items(new_capacity);  // может бросить исключение
        size_t copy_size = std::min(new_capacity, size_);
        std::copy(items_.Get(), items_.Get() + copy_size, new_items.Get());  // может бросить исключение
        return ItemsPtr(new_items.Release());
    }
    
    ArrayPtr<Type> items_{};
    size_t size_ = 0;
    size_t capacity_ = 0;
};
 
 
template <typename Type>
inline bool operator==(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return (lhs.GetSize() == rhs.GetSize())
           && std::equal(lhs.begin(), lhs.end(), rhs.begin());  // может бросить исключение
}

template <typename Type>
inline bool operator!=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(lhs == rhs);  // может бросить исключение
}

template <typename Type>
inline bool operator<(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());  // может бросить исключение
}

template <typename Type>
inline bool operator<=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return !(rhs < lhs);  // может бросить исключение
}

template <typename Type>
inline bool operator>(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs < lhs;  // может бросить исключение
}

template <typename Type>
inline bool operator>=(const SimpleVector<Type>& lhs, const SimpleVector<Type>& rhs) {
    return rhs <= lhs;  // может бросить исключение
}
