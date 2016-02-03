/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef BIGRECORD_HH_2677573720__H_
#define BIGRECORD_HH_2677573720__H_


#include <sstream>
#include "boost/any.hpp"
#include "Specific.hh"
#include "Encoder.hh"
#include "Decoder.hh"

namespace testgen {
struct Nested {
    double inval1;
    std::string inval2;
    int32_t inval3;
    Nested() :
        inval1(double()),
        inval2(std::string()),
        inval3(int32_t())
        { }
};

enum ExampleEnum {
    zero,
    one,
    two,
    three,
};

struct _bigrecord_Union__0__ {
private:
    size_t idx_;
    boost::any value_;
public:
    size_t idx() const { return idx_; }
    bool is_null() const {
        return (idx_ == 0);
    }
    void set_null() {
        idx_ = 0;
        value_ = boost::any();
    }
    std::map<std::string, int32_t > get_map() const;
    void set_map(const std::map<std::string, int32_t >& v);
    float get_float() const;
    void set_float(const float& v);
    _bigrecord_Union__0__();
};

struct _bigrecord_Union__1__ {
private:
    size_t idx_;
    boost::any value_;
public:
    size_t idx() const { return idx_; }
    std::vector<uint8_t> get_bytes() const;
    void set_bytes(const std::vector<uint8_t>& v);
    bool is_null() const {
        return (idx_ == 1);
    }
    void set_null() {
        idx_ = 1;
        value_ = boost::any();
    }
    _bigrecord_Union__1__();
};

struct RootRecord {
    int64_t mylong;
    Nested nestedrecord;
    std::map<std::string, int32_t > mymap;
    std::map<std::string, Nested > recordmap;
    std::vector<double > myarray;
    ExampleEnum myenum;
    _bigrecord_Union__0__ myunion;
    _bigrecord_Union__1__ anotherunion;
    bool mybool;
    Nested anothernested;
    boost::array<uint8_t, 16> myfixed;
    int32_t anotherint;
    std::vector<uint8_t> bytes;
    RootRecord() :
        mylong(int64_t()),
        nestedrecord(Nested()),
        mymap(std::map<std::string, int32_t >()),
        recordmap(std::map<std::string, Nested >()),
        myarray(std::vector<double >()),
        myenum(ExampleEnum()),
        myunion(_bigrecord_Union__0__()),
        anotherunion(_bigrecord_Union__1__()),
        mybool(bool()),
        anothernested(Nested()),
        myfixed(boost::array<uint8_t, 16>()),
        anotherint(int32_t()),
        bytes(std::vector<uint8_t>())
        { }
};

inline
std::map<std::string, int32_t > _bigrecord_Union__0__::get_map() const {
    if (idx_ != 1) {
        throw avro::Exception("Invalid type for union");
    }
    return boost::any_cast<std::map<std::string, int32_t > >(value_);
}

inline
void _bigrecord_Union__0__::set_map(const std::map<std::string, int32_t >& v) {
    idx_ = 1;
    value_ = v;
}

inline
float _bigrecord_Union__0__::get_float() const {
    if (idx_ != 2) {
        throw avro::Exception("Invalid type for union");
    }
    return boost::any_cast<float >(value_);
}

inline
void _bigrecord_Union__0__::set_float(const float& v) {
    idx_ = 2;
    value_ = v;
}

inline
std::vector<uint8_t> _bigrecord_Union__1__::get_bytes() const {
    if (idx_ != 0) {
        throw avro::Exception("Invalid type for union");
    }
    return boost::any_cast<std::vector<uint8_t> >(value_);
}

inline
void _bigrecord_Union__1__::set_bytes(const std::vector<uint8_t>& v) {
    idx_ = 0;
    value_ = v;
}

inline _bigrecord_Union__0__::_bigrecord_Union__0__() : idx_(0) { }
inline _bigrecord_Union__1__::_bigrecord_Union__1__() : idx_(0), value_(std::vector<uint8_t>()) { }
}
namespace avro {
template<> struct codec_traits<testgen::Nested> {
    static void encode(Encoder& e, const testgen::Nested& v) {
        avro::encode(e, v.inval1);
        avro::encode(e, v.inval2);
        avro::encode(e, v.inval3);
    }
    static void decode(Decoder& d, testgen::Nested& v) {
        if (avro::ResolvingDecoder *rd =
            dynamic_cast<avro::ResolvingDecoder *>(&d)) {
            const std::vector<size_t> fo = rd->fieldOrder();
            for (std::vector<size_t>::const_iterator it = fo.begin();
                it != fo.end(); ++it) {
                switch (*it) {
                case 0:
                    avro::decode(d, v.inval1);
                    break;
                case 1:
                    avro::decode(d, v.inval2);
                    break;
                case 2:
                    avro::decode(d, v.inval3);
                    break;
                default:
                    break;
                }
            }
        } else {
            avro::decode(d, v.inval1);
            avro::decode(d, v.inval2);
            avro::decode(d, v.inval3);
        }
    }
};

template<> struct codec_traits<testgen::ExampleEnum> {
    static void encode(Encoder& e, testgen::ExampleEnum v) {
		if (v < testgen::zero || v > testgen::three)
		{
			std::ostringstream error;
			error << "enum value " << v << " is out of bound for testgen::ExampleEnum and cannot be encoded";
			throw avro::Exception(error.str());
		}
        e.encodeEnum(v);
    }
    static void decode(Decoder& d, testgen::ExampleEnum& v) {
		size_t index = d.decodeEnum();
		if (index < testgen::zero || index > testgen::three)
		{
			std::ostringstream error;
			error << "enum value " << index << " is out of bound for testgen::ExampleEnum and cannot be decoded";
			throw avro::Exception(error.str());
		}
        v = static_cast<testgen::ExampleEnum>(index);
    }
};

template<> struct codec_traits<testgen::_bigrecord_Union__0__> {
    static void encode(Encoder& e, testgen::_bigrecord_Union__0__ v) {
        e.encodeUnionIndex(v.idx());
        switch (v.idx()) {
        case 0:
            e.encodeNull();
            break;
        case 1:
            avro::encode(e, v.get_map());
            break;
        case 2:
            avro::encode(e, v.get_float());
            break;
        }
    }
    static void decode(Decoder& d, testgen::_bigrecord_Union__0__& v) {
        size_t n = d.decodeUnionIndex();
        if (n >= 3) { throw avro::Exception("Union index too big"); }
        switch (n) {
        case 0:
            d.decodeNull();
            v.set_null();
            break;
        case 1:
            {
                std::map<std::string, int32_t > vv;
                avro::decode(d, vv);
                v.set_map(vv);
            }
            break;
        case 2:
            {
                float vv;
                avro::decode(d, vv);
                v.set_float(vv);
            }
            break;
        }
    }
};

template<> struct codec_traits<testgen::_bigrecord_Union__1__> {
    static void encode(Encoder& e, testgen::_bigrecord_Union__1__ v) {
        e.encodeUnionIndex(v.idx());
        switch (v.idx()) {
        case 0:
            avro::encode(e, v.get_bytes());
            break;
        case 1:
            e.encodeNull();
            break;
        }
    }
    static void decode(Decoder& d, testgen::_bigrecord_Union__1__& v) {
        size_t n = d.decodeUnionIndex();
        if (n >= 2) { throw avro::Exception("Union index too big"); }
        switch (n) {
        case 0:
            {
                std::vector<uint8_t> vv;
                avro::decode(d, vv);
                v.set_bytes(vv);
            }
            break;
        case 1:
            d.decodeNull();
            v.set_null();
            break;
        }
    }
};

template<> struct codec_traits<testgen::RootRecord> {
    static void encode(Encoder& e, const testgen::RootRecord& v) {
        avro::encode(e, v.mylong);
        avro::encode(e, v.nestedrecord);
        avro::encode(e, v.mymap);
        avro::encode(e, v.recordmap);
        avro::encode(e, v.myarray);
        avro::encode(e, v.myenum);
        avro::encode(e, v.myunion);
        avro::encode(e, v.anotherunion);
        avro::encode(e, v.mybool);
        avro::encode(e, v.anothernested);
        avro::encode(e, v.myfixed);
        avro::encode(e, v.anotherint);
        avro::encode(e, v.bytes);
    }
    static void decode(Decoder& d, testgen::RootRecord& v) {
        if (avro::ResolvingDecoder *rd =
            dynamic_cast<avro::ResolvingDecoder *>(&d)) {
            const std::vector<size_t> fo = rd->fieldOrder();
            for (std::vector<size_t>::const_iterator it = fo.begin();
                it != fo.end(); ++it) {
                switch (*it) {
                case 0:
                    avro::decode(d, v.mylong);
                    break;
                case 1:
                    avro::decode(d, v.nestedrecord);
                    break;
                case 2:
                    avro::decode(d, v.mymap);
                    break;
                case 3:
                    avro::decode(d, v.recordmap);
                    break;
                case 4:
                    avro::decode(d, v.myarray);
                    break;
                case 5:
                    avro::decode(d, v.myenum);
                    break;
                case 6:
                    avro::decode(d, v.myunion);
                    break;
                case 7:
                    avro::decode(d, v.anotherunion);
                    break;
                case 8:
                    avro::decode(d, v.mybool);
                    break;
                case 9:
                    avro::decode(d, v.anothernested);
                    break;
                case 10:
                    avro::decode(d, v.myfixed);
                    break;
                case 11:
                    avro::decode(d, v.anotherint);
                    break;
                case 12:
                    avro::decode(d, v.bytes);
                    break;
                default:
                    break;
                }
            }
        } else {
            avro::decode(d, v.mylong);
            avro::decode(d, v.nestedrecord);
            avro::decode(d, v.mymap);
            avro::decode(d, v.recordmap);
            avro::decode(d, v.myarray);
            avro::decode(d, v.myenum);
            avro::decode(d, v.myunion);
            avro::decode(d, v.anotherunion);
            avro::decode(d, v.mybool);
            avro::decode(d, v.anothernested);
            avro::decode(d, v.myfixed);
            avro::decode(d, v.anotherint);
            avro::decode(d, v.bytes);
        }
    }
};

}
#endif
