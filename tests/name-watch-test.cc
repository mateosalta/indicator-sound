
#include <gio/gio.h>
#include <gtest/gtest.h>

extern "C" {
#include "bus-watch-namespace.h"
}

class NameWatchTest : public ::testing::Test
{
	private:
		GTestDBus * testbus = NULL;

	protected:
		virtual void SetUp() {
			testbus = g_test_dbus_new(G_TEST_DBUS_NONE);
			g_test_dbus_up(testbus);
		}

		virtual void TearDown() {
			g_test_dbus_down(testbus);
			g_clear_object(&testbus);
		}
};

typedef struct {
	guint appeared;
	guint vanished;
} callback_count_t;

static void
appeared_simple_cb (GDBusConnection * bus, const gchar * name, const gchar * owner, gpointer user_data)
{
	callback_count_t * callback_count = static_cast<callback_count_t *>(user_data);
	callback_count->appeared++;
}

static void
vanished_simple_cb (GDBusConnection * bus, const gchar * name, gpointer user_data)
{
	callback_count_t * callback_count = static_cast<callback_count_t *>(user_data);
	callback_count->vanished++;
}


TEST_F(NameWatchTest, BaseWatch)
{
	callback_count_t callback_count = {0};

	guint ns_watch = bus_watch_namespace(G_BUS_TYPE_SESSION,
	                                     "com.foo",
	                                     appeared_simple_cb,
	                                     vanished_simple_cb,
	                                     &callback_count,
	                                     NULL);

	guint name1 = g_bus_own_name(G_BUS_TYPE_SESSION,
	                             "com.foo.bar",
	                             G_BUS_NAME_OWNER_FLAGS_NONE,
	                             NULL, NULL, NULL, NULL, NULL);
	guint name2 = g_bus_own_name(G_BUS_TYPE_SESSION,
	                             "com.foo.bar_too",
	                             G_BUS_NAME_OWNER_FLAGS_NONE,
	                             NULL, NULL, NULL, NULL, NULL);

	g_usleep(100000);
	while (g_main_pending())
		g_main_iteration(TRUE);
	g_usleep(100000);
	while (g_main_pending())
		g_main_iteration(TRUE);


	ASSERT_EQ(callback_count.appeared, 2);

	g_bus_unown_name(name1);
	g_bus_unown_name(name2);

	g_usleep(100000);
	while (g_main_pending())
		g_main_iteration(TRUE);

	ASSERT_EQ(callback_count.vanished, 2);

	bus_unwatch_namespace(ns_watch);
}

TEST_F(NameWatchTest, NonMatches)
{
	callback_count_t callback_count = {0};

	guint ns_watch = bus_watch_namespace(G_BUS_TYPE_SESSION,
	                                     "com.foo",
	                                     appeared_simple_cb,
	                                     vanished_simple_cb,
	                                     &callback_count,
	                                     NULL);

	guint name1 = g_bus_own_name(G_BUS_TYPE_SESSION,
	                             "com.foobar.bar",
	                             G_BUS_NAME_OWNER_FLAGS_NONE,
	                             NULL, NULL, NULL, NULL, NULL);
	guint name2 = g_bus_own_name(G_BUS_TYPE_SESSION,
	                             "com.bar.com.foo",
	                             G_BUS_NAME_OWNER_FLAGS_NONE,
	                             NULL, NULL, NULL, NULL, NULL);

	g_usleep(100000);
	while (g_main_pending())
		g_main_iteration(TRUE);
	g_usleep(100000);
	while (g_main_pending())
		g_main_iteration(TRUE);


	ASSERT_EQ(callback_count.appeared, 0);

	g_bus_unown_name(name1);
	g_bus_unown_name(name2);

	g_usleep(100000);
	while (g_main_pending())
		g_main_iteration(TRUE);

	ASSERT_EQ(callback_count.vanished, 0);

	bus_unwatch_namespace(ns_watch);
}

TEST_F(NameWatchTest, StartupNames)
{
	guint name1 = g_bus_own_name(G_BUS_TYPE_SESSION,
	                             "com.foo.bar",
	                             G_BUS_NAME_OWNER_FLAGS_NONE,
	                             NULL, NULL, NULL, NULL, NULL);

	g_usleep(100000);
	while (g_main_pending())
		g_main_iteration(TRUE);

	callback_count_t callback_count = {0};

	guint ns_watch = bus_watch_namespace(G_BUS_TYPE_SESSION,
	                                     "com.foo",
	                                     appeared_simple_cb,
	                                     vanished_simple_cb,
	                                     &callback_count,
	                                     NULL);

	g_usleep(100000);
	while (g_main_pending())
		g_main_iteration(TRUE);
	g_usleep(100000);
	while (g_main_pending())
		g_main_iteration(TRUE);
	g_usleep(100000);
	while (g_main_pending())
		g_main_iteration(TRUE);

	ASSERT_EQ(callback_count.appeared, 1);

	g_bus_unown_name(name1);

	g_usleep(100000);
	while (g_main_pending())
		g_main_iteration(TRUE);

	ASSERT_EQ(callback_count.vanished, 1);

	bus_unwatch_namespace(ns_watch);
}

