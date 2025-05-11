/* webpage to display kalman filter output
 * coordinates with ex_websock.py in this directory
 */

import * as d3 from "https://cdn.skypack.dev/d3@7";
/* json5 accepts ieee floatingpoint special values;
 * regular json excludes them (!?#)
 */
import JSON5 from "https://unpkg.com/json5@2/dist/index.min.mjs";

/* NOTE: put "export" in front of a variable/function
 *       that we want to make accessible outside this module
 */

/* for use for browser's javascript console */
globalThis.d3 = d3;
//globalThis.jparse = JSON5.parse;

/* u: document.URL */
function choose_ws_url(suffix_url)
{
    var pcol;
    var u = document.URL;

    /*
     * We open the websocket encrypted if this page came on an
     * https:// url itself, otherwise unencrypted
     */

    if (u.substring(0, 5) === "https") {
        pcol = "wss://";
        u = u.substr(8);
    } else {
        pcol = "ws://";
        if (u.substring(0, 4) === "http")
            u = u.substr(7);
    }

    u = u.split("/");

    /* + "/xxx" bit is for IE10 workaround */

    return pcol + u[0] + "/" + suffix_url;
} /*choose_ws_url*/

class Datatype {
    #typename = null;
    #nominal = null;
    /* .from_json(x) convert a value received in json format
     * to native representation.
     */
    #from_json = null;
    /* .make_scale(range) builds d3 scale object */
    #make_scale = null;

    constructor(typename, nominal, from_json, make_scale) {
	this.#typename = typename;
	this.#nominal = nominal;
	this.#from_json = from_json;
	this.#make_scale = make_scale;
    }

    typename() { return this.#typename; }
    nominal() { return this.#nominal; }
    from_json(x) { return this.#from_json(x); }
    make_scale(domain) { return this.#make_scale(domain); }
}; /*Datatype*/

class DatatypeFactory {
    static dtype_map = DatatypeFactory.make_dtype_map();

    static make_float_dtype() {
	return new Datatype("float" /*typename*/,
			    0.0 /*nominal*/,
			    (x) => { return x; } /*from_json*/,
			    (dom) => { return d3.scaleLinear().domain(dom); } /*make_scale*/
			   ); }

    static make_datetime_dtype() {
	return new Datatype("datetime" /*typename*/,
			    new Date() /*nominal*/,
			    (x) => { return new Date(x); } /*from_json*/,
			    (dom) => { return d3.scaleTime().domain(dom); } /*make_scale*/
			   ); }
    
    static make_dtype_map() {
	let retval = new Map();

	retval.set("float", DatatypeFactory.make_float_dtype());
	retval.set("datetime", DatatypeFactory.make_datetime_dtype());

	return retval;
    }

    static lookup(typename) {
	if (DatatypeFactory.dtype_map.has(typename)) {
	    return DatatypeFactory.dtype_map.get(typename);
	} else {
	    throw new Error("DatatypeFactory: typename ["
			    + typename
			    + "] found where float|datetime expected");
	}
    }
}; /*DatatypeFactory*/

/* class to extract event values for charting.
 * 'traits' because applies to separately-represented event objects
 */
class DataTraits {
    /* .x_slotlookup(ev) => x-value */
    #x_slotlookup = null;
    /* .y_slotlookup(ev) => y-value */
    #y_slotlookup = null;
    #x_datatype = null; //DatatypeFactory.lookup("datetime");
    #y_datatype = null; //DatatypeFactory.lookup("float");

    /* x_nt, y_nt:  each should be a pair [slotlookup, typename]
     * - slotname is a function :: event -> jsonvalue,
     *   that extracts an attribute from incoming event in json format
     * - typename is float|datetime
     */
    constructor(x_nt, y_nt) {
	this.#x_slotlookup = x_nt[0];
	this.#x_datatype = DatatypeFactory.lookup(x_nt[1]);
	this.#y_slotlookup = y_nt[0];
	this.#y_datatype = DatatypeFactory.lookup(y_nt[1]);
    }

    x_datatype() { return this.#x_datatype; }
    y_datatype() { return this.#y_datatype; }

    x_nominal() { return this.#x_datatype.nominal(); }
    y_nominal() { return this.#y_datatype.nominal(); }

    mapkey(data_ev) { return this.#x_slotlookup(data_ev); }
    x_value(data_ev) { return this.#x_datatype.from_json(this.#x_slotlookup(data_ev)); }
    y_value(data_ev) { return this.#y_datatype.from_json(this.#y_slotlookup(data_ev)); }

    make_x_scale(domain) { return this.#x_datatype.make_scale(domain); }
    make_y_scale(domain) { return this.#y_datatype.make_scale(domain); }
}; /*DataTraits*/

function range_outer(lh, rh) {
    return [Math.min(lh[0], rh[0]),
	    Math.max(lh[1], rh[1])];
} /*range_outer*/

/* a dataset driving a chart.
 *
 * PLAN: multiple lines in the same chart
 * - makeitso dataset can contain multiple data series
 *   - give each series within a dataset its own index#
 *   - each series computes its own min/max x/y values
 *   - take union across series to get chart x/y range
 * - new class Dataset
 */
class Dataseries {
    /* normalizing transformation for event objects.
     * use to produce events {.x_value, .y_value} + key suitable for Map
     */
    #data_traits = null; //new DataTraits();
    /* .dataset_map :: string -> {key value pair}
     *    must use string as keys,  since Map uses object identity if key is Object
     */
    #dataset_map = new Map();
    /* vector of key-value pairs,  in increasing x-axis order */
    #dataset_v = [];
    /* min,max value of dataset[i].x_value */
    #dset_min_x = null;
    #dset_max_x = null;
    /* min,max value of dataset[i].y_value */
    #dset_min_y = null;
    #dset_max_y = null;

    #max_key = 0;

    constructor(data_traits) {
	this.#data_traits = data_traits;
	this.recalc_minmax();
    }

    data_traits() { return this.#data_traits; }
    dataset_v() { return this.#dataset_v; }
    x_range() { return [this.#dset_min_x, this.#dset_max_x]; }
    y_range() { return [this.#dset_min_y, this.#dset_max_y]; }

    /* data_ev must have attributes consistent with what .#data_traits expects */
    update_dataset(data_ev) {
	//console.log("Dataseries.update_dataset: data_ev=", data_ev);

	let x = this.#data_traits.x_value(data_ev);
	let y = this.#data_traits.y_value(data_ev);
	/* using this key to recognize + suppress duplicate points
	 * (e.g. if browser winds up sending multiple snapshot requests
	 *       for the same dataset)
	 */
	let mapkey = this.#data_traits.mapkey(data_ev);

	//console.log("Dataseries.update_dataset: x=", x, ", y=", y, ", mapkey=", mapkey);

	/* in map must use time strings (not Dates) as keys */
	if (this.#dataset_map.has(mapkey)) {
	    /*skip -- assuming that source is immutable */;
	} else {
	    /* kv.key is ordinal number identifying a datum.
	     * not related to mapkey,  except in so far as both work as datum ids
	     */
	    let kv = {key:     this.#max_key,
		      x_value: x,
		      y_value: y};

	    /* (reminder: js map keys need to be strings) */
	    this.#dataset_map.set(mapkey, kv);
	    this.#dataset_v.push(kv);
	    this.#max_key = this.#max_key+1;
	}
    } /*update_dataset*/

    recalc_minmax() {
	if (this.#dataset_v.length == 0) {
	    /* min,max value of dataset[i].x_value */
	    this.#dset_min_x = this.#data_traits.x_nominal();
	    this.#dset_max_x = this.#data_traits.x_nominal();
	    /* min,max value of dataset[i].y_value */
	    this.#dset_min_y = this.#data_traits.y_nominal();
	    this.#dset_max_y = this.#data_traits.y_nominal();
	} else {
	    /* min,max value of dataset[i].x_value */
	    this.#dset_min_x = d3.min(this.#dataset_v, (d) => { return d.x_value; });
	    this.#dset_max_x = d3.max(this.#dataset_v, (d) => { return d.x_value; });
	    /* min,max value of dataset[i].y_value */
	    this.#dset_min_y = d3.min(this.#dataset_v, (d) => { return d.y_value; });
	    this.#dset_max_y = d3.max(this.#dataset_v, (d) => { return d.y_value; });
	}
    } /*recalc_minmax*/

    /* note: caller should invoke .range() before using for drawing */
    make_x_scale(xrange) {
	return this.#data_traits.make_x_scale(xrange /*domain*/);
    } /*make_x_scale*/

    /* note: caller should invoke .range() before using for drawing */
    make_y_scale(yrange) {
	return this.#data_traits.make_y_scale(yrange /*domain*/);
    } /*make_y_scale*/
}; /*Dataseries*/

/* bundle multiple dataseries for charting
 * for now: can have multiple series,  but they need to be driven
 * from the same native row storage
 */
class Dataset {
    #dataseries_v = [];
    /* min/max x-values across all members of .dataseries_v */
    #outer_x_range = null;
    /* min/max y-values across all members of .dataseries_v */
    #outer_y_range = null;

    constructor(data_traits_v) {
	for (let i=0, n=data_traits_v.length; i<n; ++i) {
	    this.#dataseries_v[i] = new Dataseries(data_traits_v[i]);
	}

	this.#recalc_minmax_aux();
    }

    /* #of dataseries bundled into this dataset */
    n_dataseries() {
	return this.#dataseries_v.length;
    }

    /* fetch i'th dataseries.
     *
     * Require:
     * - 0 <= i_dataseries < .n_dataseries()
     */
    lookup_dataseries(i_dataseries) {
	if (0 <= i_dataseries && i_dataseries < this.n_dataseries()) {
	    return this.#dataseries_v[i_dataseries];
	} else {
	    throw new Error('lookup_dataseries: expected i in [0..n) i=' +  i + ' n=' + this.n_dataseries());
	}
    } /*lookup_dataseries*/

    /* range of x-values,  taken across all contained series */
    x_range() {
	return this.#outer_x_range;
    }

    /* range of y-values,  taken across all contained series */
    y_range() {
	return this.#outer_y_range;
    }

    /* update dataset for a single new row */
    update_dataset(data_ev) {
	for (let i=0, n=this.n_dataseries(); i<n; ++i) {
	    this.#dataseries_v[i].update_dataset(data_ev);
	}
    } /*update_dataset*/

    /* recalc x,y ranges for each dataseries;
     * and recalculate outer x,y range stored in this dataset
     */
    recalc_minmax() {
	let n=this.n_dataseries();

	if (n > 0) {
	    for (let i=0; i<n; ++i) {
		this.#dataseries_v[i].recalc_minmax();
	    }

	    this.#recalc_minmax_aux();
	}
    } /*recalc_minmax*/

    #recalc_minmax_aux() {
	let n = this.n_dataseries();

	if (n > 0) {
	    /* update dataset minmax (i.e. union of mim-max intervals across series) */
	    let outer_x = this.#dataseries_v[0].x_range();
	    let outer_y = this.#dataseries_v[0].y_range();

	    for (let i=1; i<n; ++i) {
		let dataseries = this.#dataseries_v[i];

		outer_x = range_outer(outer_x, dataseries.x_range());
		outer_y = range_outer(outer_y, dataseries.y_range());
	    }

	    this.#outer_x_range = outer_x;
	    this.#outer_y_range = outer_y;
	}
    } /*recalc_minmax_aux*/

    /* create d3 scale for x-values in this dataset
     * (to be used for x-values in all series)
     */
    make_x_scale() {
	/* plan: verify that all series are compatible?
	 *       e.g. all timeseries or all linear
	 */

	/* using series #0 (really,  its traits) as prototype */
	return this.#dataseries_v[0].make_x_scale(this.#outer_x_range);
    } /*make_x_scale*/

    /* create d3 scale for y-values in this dataset
     * (to be used for y-values in all series)
     */
    make_y_scale() {
	/* plan: verify that all series are compatible?
	 *       e.g. all timeseries or all linear
	 */

	/* using series #0 (really, its traits) as prototype */
	return this.#dataseries_v[0].make_y_scale(this.#outer_y_range);
    } /*make_y_scale*/

}; /*Dataset*/

/* drawing code for a line chart.
 * uses svg for the "graphed line"
 *
 * originally extracted from TimeseriesCtl.
 *
 *
 *     <------- .chart_w ------->
 *     +------------------------+ ^
 *     |                        | |
 *     |   +----------------+   | |
 *     |   |        /|      |   | |
 *     |   |       / | .pad <---> |
 *     |   |      /  \      |   | |
 *     |   | /\  /    \     |   | .chart_h
 *     |   |/  --      \  --|   | |
 *     |   |            \/  |   | |
 *     |   +----------------+   | |
 *     |                        | |
 *     +------------------------+ v
 *
 * DOM:
 *    <? id=#uls> (will attach svg element here)
 *     +- <svg>
 *         +- <g id=#x_axis class=xaxis> (d3 will draw x-axis inside, .chart_x_axis() draws)
 *         +- <g id=#y_axis class=yaxis> (d3 will draw y-axis inside, .chart_y_axis() draws)
 *         +- <g id=#pts-0 class=pts>
 *             +- <path id=#line class=line> (.chart_line_gen draws)
 *
 * may have chart with multiple series;
 * we still expect to share
 *   {.chart_x_scale, .chart_y_scale, .chart_x_axis, .chart_y_axis, .chart_svg}
 * across all series.
 * we also share .chart_line_gen,  since each series supplies normalized (x_value, y_value)
 * pairs,  and we're sharing the same (x,y) d3-scales
 */
class LineChart {
    #chart_w = 100;
    #chart_h = 100;
    #chart_pad = 10;

    /* .chart_xx variables established in .require_gui() */
    /* d3 scale for x values */
    #chart_x_scale = null;
    /* d3 scale for y values */
    #chart_y_scale = null;
    /* d3 svg-line-generator */
    #chart_line_gen = null;
    /* d3 axis for x values (bottom of chart area) */
    #chart_x_axis = null;
    /* d3 axis for y values (left of chart area) */
    #chart_y_axis = null;
    /* svg chart object (the <svg> tag above in DOM sketch)*/
    #chart_svg = null;

    constructor(w, h, pad) {
	this.#chart_w = w;
	this.#chart_h = h;
	this.#chart_pad = pad;
    }

    x_range() { return [this.#chart_pad,
			this.#chart_w - this.#chart_pad]; }
    /* note: inverting bc svg y-values increase towards bottom of screen;
     *       we want y-values to increase towards top of screen
     */
    y_range() { return [this.#chart_h - this.#chart_pad,
			this.#chart_pad]; }
	
    require_gui(parent_d3sel, dataset) {
	this.#require_x_scale(dataset);
	this.#require_y_scale(dataset);
	this.#require_linegen(); /*will use .chart_x_scale, .chart_y_scale */
	this.#require_x_axis(); /*will use .chart_x_scale*/
	this.#require_y_axis(); /*will use .chart_y_scale*/
	this.#require_svg(parent_d3sel, dataset);
    }

    /* dom element id to use for the i'th dataseries in this chart */
    series_html_id(i_dataseries) {
	return "pts-" + i_dataseries;
    }

    /* update chart for new dataset contents
     *
     * Require:
     * - .require_gui(_, dataset) has been called 
     * - #of dataseries has not changed since last call to .require_svg()
     */
    update_chart(dataset) {
	/* update d3 scales
	 * (shared across all series bundled into this dataset
	 */
	this.#rescale_chart(dataset);

	for (let i=0, n=dataset.n_dataseries(); i<n; ++i) {
	    let dataseries = dataset.lookup_dataseries(i);

	    let series_svgid = this.series_html_id(i);

	    // update dataseries (pts)
	    let line = (this
			.#chart_svg
			.select("#" + series_svgid)
			.select("#line")
			.datum(dataseries.dataset_v())
			.attr("d", this.#chart_line_gen));
	}
    } /*update_chart*/

    // ----- implementation methods -----

    /* svg translate command (a string),
     * to move x-axis from origin to location relative to chart svg object
     * (i.e. to chart top left corner)
     */
    #x_axis_translate_str() {
	/* e.g.
	 *   "translate(0,450)"
	 */
	return "translate(0," + (this.#chart_h - this.#chart_pad) + ")";
    }

    /* svg translate command (a string)
     * to move y-axis from origin to location relative to chart svg object
     * (i.e. to chart top left corner)
     */
    #y_axis_translate_str() {
	/* e.g.
	 *   "translate(50,0)"
	 */
	return "translate(" + this.#chart_pad + ",0)";
    }

    #require_x_scale(dataset) {
	if (!this.#chart_x_scale) {
	    this.#chart_x_scale = (dataset.make_x_scale()
				   .range(this.x_range())
				   .nice());
	}

	return this.#chart_x_scale;
    } /*require_x_scale*/

    #require_y_scale(dataset) {
	if (!this.#chart_y_scale) {
	    this.#chart_y_scale = (dataset.make_y_scale()
				   .range(this.y_range())
				   .nice());
	}

	return this.#chart_y_scale;
    } /*require_y_scale*/

    #require_linegen() {
	if (!this.#chart_line_gen) {
	    this.#chart_line_gen = (d3.line()
				    .x((d) => { return this.#chart_x_scale(d.x_value); })
				    .y((d) => { return this.#chart_y_scale(d.y_value); }));
	}
    } /*require_linegen*/

    #require_x_axis() {
	if (!this.#chart_x_axis) {
	    this.#chart_x_axis = (d3
				  .axisBottom()
				  .scale(this.#chart_x_scale)
				  .ticks(10));
	}
    }

    #require_y_axis() {
	if (!this.#chart_y_axis) {
	    this.#chart_y_axis = (d3
				  .axisLeft()
				  .scale(this.#chart_y_scale)
				  .ticks(10));
	}
    }

    #require_svg(parent_d3sel, dataset) {
	if (!this.#chart_svg) {
	    this.#chart_svg = (parent_d3sel // .select("#uls")
			       .append("svg")
			       .attr("width", this.#chart_w)
			       .attr("height", this.#chart_h));

	    /* svg group comprising x-axis */
	    this.#chart_svg.append("g")
		.attr("class", "xaxis")
		.attr("id", "x_axis")
		.attr("transform", this.#x_axis_translate_str())
		.call(this.#chart_x_axis);

	    /* svg group comprising y-axis */
	    this.#chart_svg.append("g")
		.attr("class", "yaxis")
		.attr("id", "y_axis")
		.attr("transform", this.#y_axis_translate_str())
		.call(this.#chart_y_axis);

	    for (let i=0, n=dataset.n_dataseries(); i<n; ++i) {
		let dataseries = dataset.lookup_dataseries(i);

		let series_svgid = this.series_html_id(i);

		/* svg group comprising chart dataseries
		 *   chart_line_gen gets invoked for each member of .datum()
		 */
		this.#chart_svg.append("g")
		    .attr("class", "pts")
		    .attr("id", series_svgid)
		    .append("path")
		    .attr("class", "line")
		    .attr("id", "line")
		    .attr("fill", "none")
		    .attr("stroke", "black")
		    .datum(dataseries.dataset_v())
		    .attr("d", this.#chart_line_gen);
	    }
	}
    }

    #rescale_chart(dataset) {
	this.#chart_x_scale.domain(dataset.x_range());
	this.#chart_y_scale.domain(dataset.y_range());

	// update x-axis
	this.#chart_svg.selectAll("#x_axis")
            .attr("transform",
		  this.#x_axis_translate_str())
	    .call(this.#chart_x_axis);

	// update y-axis
	this.#chart_svg.selectAll("#y_axis")
	    .attr("transform",
		  this.#y_axis_translate_str())
	    .call(this.#chart_y_axis);
    }

}; /*LineChart*/

class Controller {
};

/*
 * DOM:
 *    <? id=#uls> (will attach svg element here)
 *     +- <svg>
 *         +- <g id=#x_axis class=xaxis> (d3 will draw x-axis inside, .chart_x_axis() draws)
 *         +- <g id=#y_axis class=yaxis> (d3 will draw y-axis inside, .chart_y_axis() draws)
 *         +- <g id=#pts-0 class=pts>
 *             +- <path id=#line class=line> (.chart_line_gen draws)
 */
class TimeseriesCtl extends Controller {
    #dataset_uri = '';
    #dataset = null;

    #chart = new LineChart(500 /*w*/,
			   250 /*h*/,
			   50 /*pad*/);

    constructor(dataset_uri, data_traits_v) {
	super();
	this.#dataset_uri = dataset_uri;
	this.#dataset = new Dataset(data_traits_v);
    }

    static rescale_dataset(dataset) {
	dataset.recalc_minmax();
    } /*rescale_dataset*/

    /* request dataseries snapshot from webserver;
     * update+draw graph when snapshot arrives
     *
     * NOTE:
     * 1. typical web docs (e.g. MDN) will advise using response.json():
     *      fetch(uri)
     *        .then((response) => response.json())
     *        .then((data) => dostuffwith(data))
     *     
     *    however,  this has a flaw: standard json is missing special floating-point values (!!);
     *    in particular it has no representation for nan/+inf/-inf
     * 2. we want to use the extended json standard 'json5';
     *    however need care since JSON5.parse() fails spuriously (at least JSON5/chrome asof 24sep2022)
     *    if given a promise
     */
    request() {
	fetch(this.#dataset_uri)
	    .then((response) => response.text())
	    .then((text) => this.on_snapshot_text(text));

//	    .then((text) => JSON5.parse()
//	    .then((data) => this.on_snapshot(data));
    } /*request*/

    /* update from snapshot json text */
    on_snapshot_text(text) {
	const data = JSON5.parse(text);

	this.on_snapshot(data);
    } /*on_snapshot_text*/

    /* update from snapshot
     *
     *  .on_snapshot() => .#dataset => .on_dataset()
     */
    on_snapshot(data) {
	//console.log("on_snapshot: data=", data);

	data.forEach((x, i) => {
	    // REFACTORME

	    if (x._name_ == "UpxEvent") {
		this.on_update(x);
	    } else if (x._name_ == "KalmanFilterStateExt") {
		this.on_update(x);
	    } else {
		console.log("unexpected json record x=", x);
	    }
	});

	this.on_dataset(this.#dataset);
    } /*on_snapshot*/

    /* update from websocket
     *
     *  .on_update() => .#dataset => .on_dataset()
     */
    on_update(data_ev) {
	this.#dataset.update_dataset(data_ev);

	this.on_dataset(this.#dataset);
    } /*on_update*/

    /* call after modifying .#dataset
     *
     *  .on_dataset() =|=> .rescale_dataset() =|========> .chart_x_axis ===\
     *                 |                       |========> .chart_x_axis =\ |
     *                 |                                                 | |
     *                 |=> .chart_svg.#x_axis <==========================/ |
     *                 |=> .chart_svg.#y_axis <============================/
     */
    on_dataset(dataset) {
	//console.log("on_dataset: dataset=", dataset);

	// update x-scale, y-scale
	TimeseriesCtl.rescale_dataset(dataset);
    
	this.#chart.update_chart(dataset);
    } /*on_dataset*/

    /* e.g.
     *   ctl.require_gui(d3.select("#uls"))
     * to build chart gui under DOM element with id="uls"
     */
    require_gui(parent_d3sel) {
	this.#chart.require_gui(parent_d3sel, this.#dataset);
    } /*require_gui*/

}; /*TimeseriesCtl*/

/* controller for timeseries graph,  from uri [/dyn/uls/snap] + [/ws/uls] */
var uls_ctl = false;
var uls_ctl_enabled = true;

if (uls_ctl_enabled) {
    uls_ctl = new TimeseriesCtl('/dyn/uls/snap',
				[new DataTraits([(ev) => ev.tm, "datetime"],
						[(ev) => ev.upx, "float"])]);

    uls_ctl.require_gui(d3.select("#uls"));
    uls_ctl.request();
}

/* controller for timeseries graph,  from uri [/dyn/kfs/snap] + [/ws/kfs] */
var kfs_ctl = false;
var kfs_ctl_enabled = true;

if (kfs_ctl_enabled) {
    kfs_ctl = new TimeseriesCtl('/dyn/kfs/snap',
				[new DataTraits([(ev) => ev.tk,
						 "datetime"],
						[(ev) => { return ev.x[0]; },
						 "float"]),
				 /* 2.sigma below estimate */
				 new DataTraits([(ev) => ev.tk,
						 "datetime"],
						[(ev) => { return Math.max(0.0, ev.x[0] - 2.0 * Math.sqrt(ev.P[0][0])); },
						 "float"]),
				 /* 2.sigma above estimate */
				 new DataTraits([(ev) => ev.tk,
						 "datetime"],
						[(ev) => { return Math.min(1.0, ev.x[0] + 2.0 * Math.sqrt(ev.P[0][0])); },
						 "float"])
				]);
    
    kfs_ctl.require_gui(d3.select("#kfs"));
    kfs_ctl.request();
}

let key_fn = ((d) => { return d.key; });

/* controller for volsurface graph (strike -> volatility),
 * from uri [/dyn/kf/snap]
 */

d3.select("#refresh")
    .on("click",
	function() {
	    console.log("button[#refresh] clicked");

	    uls_ctl.request();
	});

var srv_ws = null;

function content_loaded_fn()
{
    console.log("Hi Roly, DOM loaded");

    /* use this url to create websocket to the server that delivered current webpage */
    let ws_url = choose_ws_url("" /*url_suffix*/);
    console.log("ws_url: [", ws_url, "]");

    srv_ws = new WebSocket(ws_url, "lws-minimal");

    try {
	// srv_ws.onopen = () => { ... };
	// srv_ws.onclose = () => { ... };
	srv_ws.onmessage = (msg) => {
	    /* msg has dozens of attributes,  too many to list here
	     * actual application message appears in the .data attribute
	     * (as nested js string)
	     */
	    //console.log("incoming ws msg: [", msg, "]");
	    
	    let msgdata = JSON5.parse(msg.data);

	    //console.log("msgdata: [", msgdata, "]");

	    let stream_name = msgdata.stream;
	    let event = msgdata.event;

	    if (stream_name == "/ws/uls") {
		if (uls_ctl) {
		    uls_ctl.on_update(event);
		}
	    } else if (stream_name == "/ws/kfs") {
		if (kfs_ctl) {
		    kfs_ctl.on_update(event);
		}
	    } else {
		console.log("unknown stream name [", stream_name, "]");
	    }
	};
    } catch(excetpion) {
	asert("<p>Error: " + exception);
    }

    console.log("srv_ws state [", srv_ws.readyState, "]");
    
    srv_ws.addEventListener('open',
			    (event) => {
				console.log("srv_ws state [", srv_ws.readyState, "]");
				if (uls_ctl) {
				    srv_ws.send("{\"cmd\": \"subscribe\", \"stream\": \"/ws/uls\"} ");
				}
				if (kfs_ctl) {
				    srv_ws.send("{\"cmd\": \"subscribe\", \"stream\": \"/ws/kfs\"} ");
				}
				//socket.send('Hello Server!');
			    });
				  
} /*content_loaded_fn*/

document.addEventListener("DOMContentLoaded",
			  content_loaded_fn,
			  false);

/* end ex_websock.js */
