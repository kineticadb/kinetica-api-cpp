#include "gpudb/GenericRecord.hpp"

namespace gpudb
{
    GenericRecord::GenericRecord(const Type& type) :
        type(type)
    {
        size_t count = type.getColumnCount();
        values.reserve(count);

        for (size_t i = 0; i < count; ++i)
        {
            switch (type.getColumn(i).getType())
            {
                case ::avro::AVRO_BYTES:
                    values.push_back(std::vector<uint8_t>());
                    break;

                case ::avro::AVRO_DOUBLE:
                    values.push_back(double());
                    break;

                case ::avro::AVRO_FLOAT:
                    values.push_back(float());
                    break;

                case ::avro::AVRO_INT:
                    values.push_back(int32_t());
                    break;

                case ::avro::AVRO_LONG:
                    values.push_back(int64_t());
                    break;

                case ::avro::AVRO_STRING:
                    values.push_back(std::string());
                    break;

                default:
                    throw std::invalid_argument("Column " + type.getColumn(i).getName() + " must be of type AVRO_BYTES, AVRO_DOUBLE, AVRO_FLOAT, AVRO_INT, AVRO_LONG or AVRO_STRING.");
            }
        }
    }

    const Type& GenericRecord::getType() const
    {
        return type;
    }

    const ::avro::ValidSchema& GenericRecord::getSchema() const
    {
        return type.getSchema();
    }

    std::vector<uint8_t>& GenericRecord::asBytes(const size_t index)
    {
        return *boost::any_cast<std::vector<uint8_t> >(&values[index]);
    }

    const std::vector<uint8_t>& GenericRecord::asBytes(const size_t index) const
    {
        return *boost::any_cast<std::vector<uint8_t> >(&values[index]);
    }

    std::vector<uint8_t>& GenericRecord::asBytes(const std::string& name)
    {
        return *boost::any_cast<std::vector<uint8_t> >(&values[type.getColumnIndex(name)]);
    }

    const std::vector<uint8_t>& GenericRecord::asBytes(const std::string& name) const
    {
        return *boost::any_cast<std::vector<uint8_t> >(&values[type.getColumnIndex(name)]);
    }

    double& GenericRecord::asDouble(const size_t index)
    {
        return *boost::any_cast<double>(&values[index]);
    }

    const double& GenericRecord::asDouble(const size_t index) const
    {
        return *boost::any_cast<double>(&values[index]);
    }

    double& GenericRecord::asDouble(const std::string& name)
    {
        return *boost::any_cast<double>(&values[type.getColumnIndex(name)]);
    }

    const double& GenericRecord::asDouble(const std::string& name) const
    {
        return *boost::any_cast<double>(&values[type.getColumnIndex(name)]);
    }

    float& GenericRecord::asFloat(const size_t index)
    {
        return *boost::any_cast<float>(&values[index]);
    }

    const float& GenericRecord::asFloat(const size_t index) const
    {
        return *boost::any_cast<float>(&values[index]);
    }

    float& GenericRecord::asFloat(const std::string& name)
    {
        return *boost::any_cast<float>(&values[type.getColumnIndex(name)]);
    }

    const float& GenericRecord::asFloat(const std::string& name) const
    {
        return *boost::any_cast<float>(&values[type.getColumnIndex(name)]);
    }

    int32_t& GenericRecord::asInt(const size_t index)
    {
        return *boost::any_cast<int32_t>(&values[index]);
    }

    const int32_t& GenericRecord::asInt(const size_t index) const
    {
        return *boost::any_cast<int32_t>(&values[index]);
    }

    int32_t& GenericRecord::asInt(const std::string& name)
    {
        return *boost::any_cast<int32_t>(&values[type.getColumnIndex(name)]);
    }

    const int32_t& GenericRecord::asInt(const std::string& name) const
    {
        return *boost::any_cast<int32_t>(&values[type.getColumnIndex(name)]);
    }

    int64_t& GenericRecord::asLong(const size_t index)
    {
        return *boost::any_cast<int64_t>(&values[index]);
    }

    const int64_t& GenericRecord::asLong(const size_t index) const
    {
        return *boost::any_cast<int64_t>(&values[index]);
    }

    int64_t& GenericRecord::asLong(const std::string& name)
    {
        return *boost::any_cast<int64_t>(&values[type.getColumnIndex(name)]);
    }

    const int64_t& GenericRecord::asLong(const std::string& name) const
    {
        return *boost::any_cast<int64_t>(&values[type.getColumnIndex(name)]);
    }

    std::string& GenericRecord::asString(const size_t index)
    {
        return *boost::any_cast<std::string>(&values[index]);
    }

    const std::string& GenericRecord::asString(const size_t index) const
    {
        return *boost::any_cast<std::string>(&values[index]);
    }

    std::string& GenericRecord::asString(const std::string& name)
    {
        return *boost::any_cast<std::string>(&values[type.getColumnIndex(name)]);
    }

    const std::string& GenericRecord::asString(const std::string& name) const
    {
        return *boost::any_cast<std::string>(&values[type.getColumnIndex(name)]);
    }
}

namespace avro
{
    void codec_traits<gpudb::GenericRecord>::encode(Encoder& e, const gpudb::GenericRecord& g)
    {
        NodePtr root = g.getSchema().root();
        size_t c = root->leaves();

        for (size_t i = 0; i < c; ++i)
        {
            switch (root->leafAt(i)->type())
            {
                case AVRO_BYTES:
                    e.encodeBytes(g.asBytes(i));
                    break;

                case AVRO_DOUBLE:
                    e.encodeDouble(g.asDouble(i));
                    break;

                case AVRO_FLOAT:
                    e.encodeFloat(g.asFloat(i));
                    break;

                case AVRO_INT:
                    e.encodeInt(g.asInt(i));
                    break;

                case AVRO_LONG:
                    e.encodeLong(g.asLong(i));
                    break;

                case AVRO_STRING:
                    e.encodeString(g.asString(i));
                    break;

                default:
                    throw std::invalid_argument("Field " + root->nameAt(i) + " must be of type AVRO_BYTES, AVRO_DOUBLE, AVRO_FLOAT, AVRO_INT, AVRO_LONG or AVRO_STRING.");
            }
        }
    }

    void codec_traits<gpudb::GenericRecord>::decode(Decoder& d, gpudb::GenericRecord& g)
    {
        NodePtr root = g.getSchema().root();
        size_t c = root->leaves();

        if (::avro::ResolvingDecoder *rd = dynamic_cast< ::avro::ResolvingDecoder*>(&d))
        {
            const std::vector<size_t> fo = rd->fieldOrder();

            for (size_t i = 0; i < c; ++i)
            {
                switch (root->leafAt(fo[i])->type())
                {
                    case AVRO_BYTES:
                        g.asBytes(fo[i]) = d.decodeBytes();
                        break;

                    case AVRO_DOUBLE:
                        g.asDouble(fo[i]) = d.decodeDouble();
                        break;

                    case AVRO_FLOAT:
                        g.asFloat(fo[i]) = d.decodeFloat();
                        break;

                    case AVRO_INT:
                        g.asInt(fo[i]) = d.decodeInt();
                        break;

                    case AVRO_LONG:
                        g.asLong(fo[i]) = d.decodeLong();
                        break;

                    case AVRO_STRING:
                        g.asString(fo[i]) = d.decodeString();
                        break;

                    default:
                        throw std::invalid_argument("Field " + root->nameAt(i) + " must be of type AVRO_BYTES, AVRO_DOUBLE, AVRO_FLOAT, AVRO_INT, AVRO_LONG or AVRO_STRING.");
                }
            }
        }
        else
        {
            for (size_t i = 0; i < c; ++i)
            {
                switch (root->leafAt(i)->type())
                {
                    case AVRO_BYTES:
                        g.asBytes(i) = d.decodeBytes();
                        break;

                    case AVRO_DOUBLE:
                        g.asDouble(i) = d.decodeDouble();
                        break;

                    case AVRO_FLOAT:
                        g.asFloat(i) = d.decodeFloat();
                        break;

                    case AVRO_INT:
                        g.asInt(i) = d.decodeInt();
                        break;

                    case AVRO_LONG:
                        g.asLong(i) = d.decodeLong();
                        break;

                    case AVRO_STRING:
                        g.asString(i) = d.decodeString();
                        break;

                    default:
                        throw std::invalid_argument("Field " + root->nameAt(i) + " must be of type AVRO_BYTES, AVRO_DOUBLE, AVRO_FLOAT, AVRO_INT, AVRO_LONG or AVRO_STRING.");
                }
            }
        }
    }
}