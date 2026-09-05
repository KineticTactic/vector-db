#include <any>
#include <cstdint>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <vecdb/vector_record.hpp>

TEST(VectorRecordTest, DimensionMatchesVectorSize) {
    const vecdb::VectorRecord<float> record{0, {1.0f, 2.0f, 3.0f}, {}};

    EXPECT_EQ(record.id, 0);
    EXPECT_EQ(record.dimension(), 3u);
}

TEST(VectorRecordTest, EmptyVectorHasZeroDimension) {
    const vecdb::VectorRecord<float> record{7, {}, {}};

    EXPECT_EQ(record.dimension(), 0u);
}

// The same record type has to serve .fvecs data and .ivecs ground-truth
// indices, so it must instantiate for both element types.
TEST(VectorRecordTest, HoldsInt32Vectors) {
    const vecdb::VectorRecord<std::int32_t> record{1, {7, 8, 9, 10}, {}};

    EXPECT_EQ(record.dimension(), 4u);
    EXPECT_EQ(record.vector[2], 9);
}

TEST(VectorRecordTest, MetadataIsEmptyUntilPopulated) {
    const vecdb::VectorRecord<float> record{0, {1.0f}, {}};

    EXPECT_TRUE(record.metadata.fields.empty());
}

// std::any is what lets one metadata map hold values of unrelated types.
TEST(MetadataTest, StoresValuesOfDifferentTypes) {
    vecdb::Metadata metadata;
    metadata.fields["filename"] = std::string("siftsmall_base.fvecs");
    metadata.fields["page_number"] = 42;

    EXPECT_EQ(metadata.fields.size(), 2u);
    EXPECT_EQ(std::any_cast<std::string>(metadata.fields.at("filename")), "siftsmall_base.fvecs");
    EXPECT_EQ(std::any_cast<int>(metadata.fields.at("page_number")), 42);
    EXPECT_THROW((void)std::any_cast<float>(metadata.fields.at("page_number")), std::bad_any_cast);
}
