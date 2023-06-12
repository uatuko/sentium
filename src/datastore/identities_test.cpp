#include <gtest/gtest.h>

#include "err/errors.h"

#include "identities.h"
#include "testing.h"

class IdentitiesTest : public ::testing::Test {
protected:
	static void SetUpTestSuite() {
		datastore::testing::setup();

		// Clear data
		datastore::pg::exec("truncate table identities cascade;");
	}

	void SetUp() {
		// Clear data before each test
		datastore::pg::exec("delete from identities cascade;");
	}

	static void TearDownTestSuite() { datastore::testing::teardown(); }
};

TEST_F(IdentitiesTest, discard) {
	const datastore::Identity identity({
		.sub = "sub:IdentitiesTest.discard",
	});

	EXPECT_NO_THROW(identity.store());
	EXPECT_NO_THROW(identity.discard());

	std::string_view qry = R"(
		select
			count(*)
		from identities
		where _id = $1::text;
	)";

	auto res   = datastore::pg::exec(qry, identity.id());
	auto count = res.at(0, 0).as<int>();
	EXPECT_EQ(0, count);
}

TEST_F(IdentitiesTest, retrieve) {
	// Success: retrieve data
	{
		std::string_view qry = R"(
			insert into identities (
				_id,
				_rev,
				sub
			) values (
				$1::text,
				$2::integer,
				$3::text
			);
		)";

		try {
			datastore::pg::exec(
				qry, "_id:IdentitiesTest.retrieve", 2308, "sub:IdentitiesTest.retrieve");
		} catch (const std::exception &e) {
			FAIL() << e.what();
		}

		auto identity = datastore::RetrieveIdentity("_id:IdentitiesTest.retrieve");
		EXPECT_EQ(2308, identity.rev());
		EXPECT_EQ("sub:IdentitiesTest.retrieve", identity.sub());
	}

	// Error: not found
	{ EXPECT_THROW(datastore::RetrieveIdentity("_id:dummy"), err::DatastoreIdentityNotFound); }
}

TEST_F(IdentitiesTest, rev) {
	// Success: revision increment
	{
		const datastore::Identity identity({
			.sub = "sub:IdentitiesTest.rev",
		});

		EXPECT_NO_THROW(identity.store());
		EXPECT_EQ(0, identity.rev());

		EXPECT_NO_THROW(identity.store());
		EXPECT_EQ(1, identity.rev());
	}

	// Error: revision mismatch
	{
		const datastore::Identity identity({
			.sub = "sub:IdentitiesTest.rev-mismatch",
		});

		std::string_view qry = R"(
			insert into identities (
				_id,
				_rev,
				sub
			) values (
				$1::text,
				$2::integer,
				$3::text
			)
		)";

		EXPECT_NO_THROW(
			datastore::pg::exec(qry, identity.id(), identity.rev() + 1, identity.sub()));

		EXPECT_THROW(identity.store(), err::DatastoreRevisionMismatch);
	}
}

TEST_F(IdentitiesTest, store) {
	const datastore::Identity identity({
		.sub = "sub:IdentitiesTest.store",
	});

	// Success: persist data
	{
		EXPECT_NO_THROW(identity.store());

		std::string_view qry = R"(
			select
				_id,
				_rev,
				sub
			from identities
			where _id = $1::text;
		)";

		auto res = datastore::pg::exec(qry, identity.id());
		EXPECT_EQ(1, res.size());

		auto [_id, _rev, sub] = res[0].as<std::string, int, std::string>();
		EXPECT_EQ(identity.id(), _id);
		EXPECT_EQ(identity.rev(), _rev);
		EXPECT_EQ(identity.sub(), sub);
	}

	// Error: duplicate `sub`
	{
		const datastore::Identity duplicate({
			.sub = identity.sub(),
		});

		EXPECT_THROW(duplicate.store(), err::DatastoreDuplicateIdentity);
	}
}
